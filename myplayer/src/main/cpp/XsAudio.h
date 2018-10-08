//
// Created by zxsong on 2018/9/19.
//

#ifndef MYMUSIC_XSAUDIO_H
#define MYMUSIC_XSAUDIO_H

#include "XsPlaystatus.h"
#include "XsQueue.h"
#include "XsCallJava.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};

class XsAudio {
public:
    int streamIndex = -1;
    AVCodecContext *avCodecCtx = NULL;
    AVCodecParameters *avCodecPar = NULL;
    XsPlaystatus *playstatus = NULL;
    XsQueue *queue = NULL;
    XsCallJava *callJava = NULL;

    pthread_t playThread;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = 0;
    uint8_t *buffer = NULL;
    int data_size = 0;
    int sample_rate = 0;

    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

public:
    XsAudio(XsPlaystatus *playstatus, int sample_rate, XsCallJava *callJava1);
    ~XsAudio();

    void play();
    int resampleAudio();

    void initOpenSLES();

    int getSampleRateForOpenSLES(int sample_rate);

    void pause();

    void resume();
};


#endif //MYMUSIC_XSAUDIO_H
