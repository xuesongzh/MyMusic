//
// Created by zxsong on 2018/9/19.
//

#include "XsAudio.h"

XsAudio::XsAudio(XsPlaystatus *playstatus, int sample_rate, XsCallJava *callJava) {
    this->playStatus = playstatus;
    this->sample_rate = sample_rate;
    this->callJava = callJava;
    queue = new XsQueue(playstatus);
    buffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);//1s的PCM大小
    pthread_mutex_init(&codecMutex, NULL);
}

XsAudio::~XsAudio() {
    pthread_mutex_destroy(&codecMutex);
}

void *playAudio(void *data) {
    XsAudio *xsAudio = (XsAudio *) data;
    xsAudio->initOpenSLES();
//    pthread_exit(&xsAudio->playThread);
    return nullptr;
}


void XsAudio::play() {
    if (playStatus !=NULL && !playStatus->exit) {
        pthread_create(&playThread, NULL, playAudio, this);
    }
}

//FILE *outFile = fopen("/storage/emulated/0/mydream.pcm", "w");

//AVframe重采样生成PCM数据（转码）
int XsAudio::resampleAudio() {

    while (playStatus != NULL && !playStatus->exit) {

        //添加加载功能
        if (queue->getQueueSize() == 0) {//加载中
            if (!playStatus->load) {
                playStatus->load = true;
                callJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000 * 100);
            continue;
        } else {
            if (playStatus->load) {
                playStatus->load = false;
                callJava->onCallLoad(CHILD_THREAD, false);
            }
        }

        avPacket = av_packet_alloc();
        if (queue->getAvpacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        pthread_mutex_lock(&codecMutex);
        ret = avcodec_send_packet(avCodecCtx, avPacket);
        if (ret != 0) {//error
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            pthread_mutex_unlock(&codecMutex);
            continue;
        }

        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecCtx, avFrame);
        if (ret == 0) {
            //判断声道布局和声道数，异常情况
            if (avFrame->channels > 0 && avFrame->channel_layout == 0) {
                avFrame->channel_layout = (uint64_t) av_get_default_channel_layout(
                        avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swrContext;
            swrContext = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,// 输出声道布局
                    AV_SAMPLE_FMT_S16,// 输出采样位数格式
                    avFrame->sample_rate,           // 输出采样率
                    avFrame->channel_layout,        // 输入声道布局
                    (AVSampleFormat) avFrame->format,//输入采样位数格式
                    avFrame->sample_rate,//输入采样率
                    0,
                    NULL);

            if (swrContext == NULL || swr_init(swrContext) < 0) {//failed
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                if (swrContext != NULL) {
                    swr_free(&swrContext);
                    swrContext = NULL;
                }
                pthread_mutex_unlock(&codecMutex);
                continue;
            }

            int nb = swr_convert(
                    swrContext,
                    &buffer,//转码后输出的PCM数据大小
                    avFrame->nb_samples,//输出采样个数
                    (const uint8_t **) avFrame->data,//原始压缩数据
                    avFrame->nb_samples);//输入采样个数

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            /*if (LOG_DEBUG) {
                LOGD("每帧的大小：%d", data_size);
            }*/

//            fwrite(buffer, 1, data_size, outFile);

            //当前AVframe时间
            now_time = avFrame->pts * av_q2d(time_base);
//            LOGD("当前AVframe时间: %f", now_time);
            if (now_time < play_time) {
                now_time = play_time;
            }
            play_time = now_time;

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);
            swrContext = NULL;
            pthread_mutex_unlock(&codecMutex);
            break;
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            pthread_mutex_unlock(&codecMutex);
            continue;
        }

    }
//    fclose(outFile);
    return data_size;
}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    XsAudio *xsAudio = (XsAudio *) context;
    if (xsAudio != NULL) {
        int bufferSize = xsAudio->resampleAudio();
        if (bufferSize > 0) {
//            xsAudio->play_time += bufferSize /  ((double) (xsAudio->sample_rate * 2 * 2));
            if (xsAudio->play_time - xsAudio->last_time >= 0.1) {//每秒回调10次
                xsAudio->last_time = xsAudio->play_time;
                xsAudio->callJava->onCallTimeInfo(CHILD_THREAD, xsAudio->play_time,
                                                  xsAudio->duration);
            }

            (*xsAudio->pcmBufferQueue)->Enqueue(xsAudio->pcmBufferQueue, (char *) xsAudio->buffer,
                                                bufferSize);
        }
    }
}

void XsAudio::initOpenSLES() {
    SLresult result;

    //第一步，创建引擎
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);

    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};


    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};

    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            (uint32_t) getSampleRateForOpenSLES(sample_rate),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};


    const SLInterfaceID ids[2] = {SL_IID_BUFFERQUEUE, SL_IID_PLAYBACKRATE};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 2,
                                       ids, req);
    // 初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

    // 得到接口后调用，获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

    // 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);

    // 注册缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);

    // 获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);

    pcmBufferCallBack(pcmBufferQueue, this);
}

int XsAudio::getSampleRateForOpenSLES(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void XsAudio::pause() {
    if (pcmPlayerPlay != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PAUSED);
    }
}

void XsAudio::resume() {
    if (pcmPlayerPlay != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    }
}

void XsAudio::stop() {
    if (pcmPlayerPlay != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
    play_time = 0;
    last_time = 0;
}

void XsAudio::release() {

    if (queue != NULL) {
        queue->notifyQueue();
    }
    pthread_join(playThread, NULL);

    //释放队列
    if (queue != NULL) {
        delete queue;
        queue = NULL;
    }

    //释放OpenSL
    if (pcmPlayerObject != NULL) {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
    }

    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    if (avCodecCtx != NULL) {
        pthread_mutex_lock(&codecMutex);
        avcodec_close(avCodecCtx);
        avcodec_free_context(&avCodecCtx);
        avCodecCtx = NULL;
        pthread_mutex_unlock(&codecMutex);
    }

    if (playStatus != NULL) {
        playStatus = NULL;
    }

    if (callJava != NULL) {
        callJava = NULL;
    }
}


