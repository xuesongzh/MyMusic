#ifndef MYMUSIC_XSVIDEO_H
#define MYMUSIC_XSVIDEO_H

#include "XsAudio.h"
#include "XsQueue.h"
#include "XsCallJava.h"

#define CODEC_YUV 0
#define CODEC_MEDIA_CODEC 1

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
    double playTime = 0;
    double delayTime = 0;
    double defaultDelayTime = 0.04; //sleep一帧时间
    pthread_mutex_t codecMutex;

    int codecType = CODEC_YUV;
    AVBSFContext *bsfCtx = NULL;


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
