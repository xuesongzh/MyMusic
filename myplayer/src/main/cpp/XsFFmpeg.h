//
// Created by zxsong on 2018/9/19.
//

#ifndef MYMUSIC_XSFFMPEG_H
#define MYMUSIC_XSFFMPEG_H

#include "XsCallJava.h"
#include "pthread.h"
#include "XsAudio.h"
#include "XsVideo.h"
#include "XsPlaystatus.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/time.h"
};

class XsFFmpeg {

public:
    XsCallJava *callJava = NULL;
    const char *url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pFormatCtx = NULL;
    XsAudio *audio = NULL;
    XsVideo *video = NULL;
    XsPlaystatus *playStatus = NULL;

    bool exit = false;
    int duration = 0;

    pthread_mutex_t init_mutex;
    pthread_mutex_t seek_mutex;

public:
    XsFFmpeg(XsPlaystatus *playStatus, XsCallJava *callJava, const char *url);
    ~XsFFmpeg();

    void parpared();
    void decodeFFmpegThread();
    void start();

    void pause();

    void resume();

    void release();

    void seek(int64_t seconds);

    void getCodecContext(AVCodecParameters *codecPar, AVCodecContext **avCodecContext);
};


#endif //MYMUSIC_XSFFMPEG_H
