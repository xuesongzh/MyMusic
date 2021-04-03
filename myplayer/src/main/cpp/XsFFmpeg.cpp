//
// Created by zxsong on 2018/9/19.
//

#include "XsFFmpeg.h"

XsFFmpeg::XsFFmpeg(XsPlaystatus *playStatus, XsCallJava *callJava, const char *url) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    this->url = url;
    pthread_mutex_init(&init_mutex, NULL);
    pthread_mutex_init(&seek_mutex, NULL);
}

XsFFmpeg::~XsFFmpeg() {
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);
}

void *decodeFFmpeg(void *data) {
    XsFFmpeg *xsFFmpeg = (XsFFmpeg *) data;
    xsFFmpeg->decodeFFmpegThread();
    pthread_exit(&xsFFmpeg->decodeThread);
}

void XsFFmpeg::parpared() {
    /* pthread_creat :
    用于创建一个实际的线程如：pthread_create(&pthread,NULL,threadCallBack,NULL);
    其总共接收4个参数，第一个参数为pthread_t对象
    第二个参数为线程的一些属性我们一般传NULL就行
    第三个参数为线程执行的函数（ void* threadCallBack(void *data) ）
    第四个参数是传递给线程的参数，是void*类型的，可以传任意类型*/
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

int avformat_callback(void *ctx) {
    XsFFmpeg *ffmpeg = (XsFFmpeg *) ctx;
    if (ffmpeg->playStatus->exit) {
        return AVERROR_EOF;
    }
    return 0;
}

void XsFFmpeg::decodeFFmpegThread() {

    pthread_mutex_lock(&init_mutex);
    //1、注册解码器并初始化网络
    av_register_all();
    avformat_network_init();

    //2、打开文件或网络流
    pFormatCtx = avformat_alloc_context();

    pFormatCtx->interrupt_callback.callback = avformat_callback;
    pFormatCtx->interrupt_callback.opaque = this;

    if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
        if (LOG_DEBUG) {
            LOGE("can not open url :%s", url);
        }
        callJava->onCallError(CHILD_THREAD, 1001, "can not open url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    //3、获取流信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not find stream from :%s", url);
        }
        callJava->onCallError(CHILD_THREAD, 1002, "can not find stream from url");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    //4、获取音频流或者视频流
    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                audio = new XsAudio(playStatus, pFormatCtx->streams[i]->codecpar->sample_rate,
                                    callJava);
                audio->streamIndex = i;
                audio->avCodecPar = pFormatCtx->streams[i]->codecpar;
                audio->duration = (int) (pFormatCtx->duration / AV_TIME_BASE);
                audio->time_base = pFormatCtx->streams[i]->time_base;
                duration = audio->duration;
            }
        } else if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video == NULL) {
                video = new XsVideo(playStatus, callJava);
                video->streamIndex = i;
                video->avCodecPar = pFormatCtx->streams[i]->codecpar;
                video->time_base = pFormatCtx->streams[i]->time_base;

                int num = pFormatCtx->streams[i]->avg_frame_rate.num;
                int den = pFormatCtx->streams[i]->avg_frame_rate.den;
                if (num != 0 && den != 0) {
                    int fps = num / den;  //[25 / 1]
                    video->defaultDelayTime = 1.0 / fps;
                }
            }
        }
    }

    if (audio != NULL) {
        getCodecContext(audio->avCodecPar, &audio->avCodecCtx);
    }

    if (video != NULL) {
        getCodecContext(video->avCodecPar, &video->avCodecCtx);
    }

    callJava->onCallPrepared(CHILD_THREAD);

    pthread_mutex_unlock(&init_mutex);
}

void XsFFmpeg::start() {
    if (audio == NULL) {
        return;
    }

    if (video == NULL) {
        return;
    }
    video->audio = audio;

    audio->play();
    video->play();

    //8、读取音视频帧
//    int count = 0;
    while (playStatus != NULL && !playStatus->exit) {
        if (playStatus->seek) {
            av_usleep(1000 * 100);
            continue;
        }

        //设置队列的大小
        if (audio->queue->getQueueSize() > 40) {
            av_usleep(1000 * 100);
            continue;
        }

        AVPacket *avPacket = av_packet_alloc();

        pthread_mutex_lock(&seek_mutex);
        int ret = av_read_frame(pFormatCtx, avPacket);
        pthread_mutex_unlock(&seek_mutex);

        if (ret == 0) {
            if (avPacket->stream_index == audio->streamIndex) {
                /*count++;
                if (LOG_DEBUG) {
                    LOGE("解码第 %d 帧", count);
                }*/
                audio->queue->putAvpacket(avPacket);
            } else if (avPacket->stream_index == video->streamIndex) {
                video->queue->putAvpacket(avPacket);
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
            }
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            while (playStatus != NULL && !playStatus->exit) {
                if (audio->queue->getQueueSize() > 0) {
                    av_usleep(1000 * 100);
                    continue;
                } else {
                    if (!playStatus->seek) {
                        av_usleep(1000 * 100);
                        playStatus->exit = true;
                    }
                    break;
                }
            }
        }
    }

    //模拟出队
//    while (audio->queue->getQueueSize() > 0) {
//        AVPacket *avPacket = av_packet_alloc();
//        audio->queue->getAvpacket(avPacket);
//        av_packet_free(&avPacket);
//        av_free(avPacket);
//        avPacket = NULL;
//    }

    if (callJava != NULL) {
        callJava->onCallComplete(CHILD_THREAD);
    }

    exit = true;

    if (LOG_DEBUG) {
        LOGE("decode finished");
    }
}

void XsFFmpeg::pause() {
    if (playStatus != NULL) {
        playStatus->pause = true;
    }

    if (audio != NULL) {
        audio->pause();
    }
}

void XsFFmpeg::resume() {
    if (playStatus != NULL) {
        playStatus->pause = false;
    }

    if (audio != NULL) {
        audio->resume();
    }
}

void XsFFmpeg::release() {

    if (LOG_DEBUG) {
        LOGE("开始释放Ffmpeg");
    }

    playStatus->exit = true;

    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    while (!exit) {
        if (sleepCount > 1000) {
            exit = true;
        }
        if (LOG_DEBUG) {
            LOGE("wait ffmpeg  exit %d", sleepCount);
        }
        sleepCount++;
        av_usleep(1000 * 10);//暂停10毫秒
    }

    if (LOG_DEBUG) {
        LOGE("释放 Audio");
    }
    if (audio != NULL) {
        audio->release();
        delete audio;
        audio = NULL;
    }

    if (LOG_DEBUG) {
        LOGE("释放 Video");
    }
    if (video != NULL) {
        video->release();
        delete video;
        video = NULL;
    }

    if (pFormatCtx != NULL) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
    }

    if (callJava != NULL) {
        callJava = NULL;
    }

    if (playStatus != NULL) {
        playStatus = NULL;
    }
    pthread_mutex_unlock(&init_mutex);
}

void XsFFmpeg::seek(int64_t seconds) {
    LOGE("seek time %ld s", seconds);
    if (duration <= 0) {
        return;
    }

    if (seconds >= 0 && seconds <= duration) {
        playStatus->seek = true;
        pthread_mutex_lock(&seek_mutex);
        int64_t pos = seconds * AV_TIME_BASE;
        avformat_seek_file(pFormatCtx, -1, INT64_MIN, pos, INT64_MAX, 0);

        if (audio != NULL) {
            audio->queue->clearAvpacket();
            audio->play_time = 0;
            audio->last_time = 0;

            pthread_mutex_lock(&audio->codecMutex);
            avcodec_flush_buffers(audio->avCodecCtx);
            pthread_mutex_unlock(&audio->codecMutex);
        }

        if (video != NULL) {
            video->queue->clearAvpacket();
            video->playTime = 0;

            pthread_mutex_lock(&video->codecMutex);
            avcodec_flush_buffers(video->avCodecCtx);
            pthread_mutex_unlock(&video->codecMutex);
        }
        pthread_mutex_unlock(&seek_mutex);
        playStatus->seek = false;
    }
}

void XsFFmpeg::getCodecContext(AVCodecParameters *codecPar, AVCodecContext **avCodecContext) {
    //5、获取解码器
    AVCodec *codec = avcodec_find_decoder(codecPar->codec_id);
    if (!codec) {
        if (LOG_DEBUG) {
            LOGE("can not find decoder");
        }
        callJava->onCallError(CHILD_THREAD, 1003, "can not find decoder");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    //6、利用解码器创建解码器上下文
    *avCodecContext = avcodec_alloc_context3(codec);
    if (!*avCodecContext) {
        if (LOG_DEBUG) {
            LOGE("can not alloc new decodecctx");
        }
        callJava->onCallError(CHILD_THREAD, 1004, "can not alloc new decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    if (avcodec_parameters_to_context(*avCodecContext, codecPar) < 0) {
        if (LOG_DEBUG) {
            LOGE("can not fill decodecctx");
        }
        callJava->onCallError(CHILD_THREAD, 1005, "can not fill decodecctx");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    //7、打开解码器
    if (avcodec_open2(*avCodecContext, codec, 0) != 0) {
        if (LOG_DEBUG) {
            LOGE("cant not open audio strames");
        }
        callJava->onCallError(CHILD_THREAD, 1006, "cant not open audio strames");
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }
    return;
}
