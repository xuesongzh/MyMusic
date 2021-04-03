#ifndef MYMUSIC_XSVIDEO_H
#define MYMUSIC_XSVIDEO_H

#include "XsAudio.h"
#include "XsQueue.h"
#include "XsCallJava.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
};

class XsVideo {
public:
    int streamIndex = -1;
    AVCodecContext *avCodecCtx = NULL;
    AVCodecParameters *avCodecPar = NULL;
    XsQueue *queue = NULL;
    XsPlaystatus *playStatus = NULL;
    XsCallJava *callJava = NULL;
    AVRational time_base;
    pthread_t thread_play;
    XsAudio *audio = NULL;
//    double clock = 0;
//    double delayTime = 0;
//    double defaultDelayTime = 0.04;
//    pthread_mutex_t codecMutex;
//
//    int codectype = CODEC_YUV;
//
//    AVBSFContext *abs_ctx = NULL;



public:
    XsVideo(XsPlaystatus *playStatus, XsCallJava *callJava);
    ~XsVideo();

    void play();

    void release();

    double getFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket);

    double getDelayTime(double diff);

    void freePacket(AVPacket *avPacket);

    void freeFrame(AVFrame *avFrame);

};


#endif //MYMUSIC_XSVIDEO_H
