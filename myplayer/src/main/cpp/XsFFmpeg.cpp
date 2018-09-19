//
// Created by zxsong on 2018/9/19.
//

#include "XsFFmpeg.h"

XsFFmpeg::XsFFmpeg(XsCallJava *callJava, const char *url) {
    this->callJava = callJava;
    this->url = url;
}

void *decodeFFmpeg(void *data) {
    XsFFmpeg *xsFFmpeg = (XsFFmpeg *) data;
    xsFFmpeg->decodeFFmpegThread();
    pthread_exit(&xsFFmpeg->decodeThread);
}
void XsFFmpeg::parpared() {
    pthread_create(&decodeThread, NULL, decodeFFmpeg, this);
}

void XsFFmpeg::decodeFFmpegThread() {
    //1、注册解码器并初始化网络
    av_register_all();
    avformat_network_init();

    //2、打开文件或网络流
    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
        if(LOG_DEBUG) {
            LOGE("can not open url :%s", url);
        }
        return;
    }

    //3、获取流信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        if(LOG_DEBUG) {
            LOGE("can not find stream from :%s", url);
        }
        return;
    }

    //4、获取音频流
    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio == NULL) {
                audio = new XsAudio();
                audio->streamIndex = i;
                audio->avCodecPar = pFormatCtx->streams[i]->codecpar;
            }
        }
    }

    //5、获取解码器
    AVCodec *codec = avcodec_find_decoder(audio->avCodecPar->codec_id);
    if (!codec) {
        if(LOG_DEBUG) {
            LOGE("can not find decoder");
        }
        return;
    }

    //6、利用解码器创建解码器上下文
    audio->avCodecCtx = avcodec_alloc_context3(codec);
    if (!audio->avCodecCtx) {
        if(LOG_DEBUG) {
            LOGE("can not alloc new decodecctx");
        }
        return;
    }
    if (avcodec_parameters_to_context(audio->avCodecCtx, audio->avCodecPar) < 0) {
        if(LOG_DEBUG) {
            LOGE("can not fill decodecctx");
        }
        return;
    }

    //7、打开解码器
    if (avcodec_open2(audio->avCodecCtx, codec, 0) != 0) {
        if(LOG_DEBUG) {
            LOGE("cant not open audio strames");
        }
        return;
    }
    callJava->onCallPrepared(CHILD_THREAD);
}

void XsFFmpeg::start() {
    if(audio == NULL) {
        if(LOG_DEBUG) {
            LOGE("audio is null");
            return;
        }
    }

    //8、读取音频帧
    int count = 0;
    while (1) {
        AVPacket *avPacket = av_packet_alloc();
        if (av_read_frame(pFormatCtx, avPacket) == 0) {
            if (avPacket->stream_index == audio->streamIndex) {
                count++;
                if (LOG_DEBUG) {
                    LOGE("解码第 %d 帧", count);
                }
                av_packet_free(&avPacket);
                av_free(avPacket);
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
            }
        } else {
            if(LOG_DEBUG) {
                LOGE("decode finished");
            }
            av_packet_free(&avPacket);
            av_free(avPacket);
            break;
        }
    }

}
