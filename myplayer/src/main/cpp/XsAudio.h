//
// Created by zxsong on 2018/9/19.
//

#ifndef MYMUSIC_XSAUDIO_H
#define MYMUSIC_XSAUDIO_H

#include "XsPlaystatus.h"
#include "XsQueue.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include <libswresample/swresample.h>
};

class XsAudio {
public:
    int streamIndex = -1;
    AVCodecContext *avCodecCtx = NULL;
    AVCodecParameters *avCodecPar = NULL;
    XsPlaystatus *playstatus = NULL;
    XsQueue *queue = NULL;

    pthread_t playThread;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = 0;
    uint8_t *buffer = NULL;
    int data_size = 0;

public:
    XsAudio(XsPlaystatus *playstatus);
    ~XsAudio();

    void play();
    int resampleAudio();
};


#endif //MYMUSIC_XSAUDIO_H
