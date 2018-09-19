//
// Created by zxsong on 2018/9/19.
//

#ifndef MYMUSIC_XSAUDIO_H
#define MYMUSIC_XSAUDIO_H

extern "C"
{
#include "libavcodec/avcodec.h"
};

class XsAudio {
public:
    int streamIndex = -1;
    AVCodecContext *avCodecCtx = NULL;
    AVCodecParameters *avCodecPar = NULL;

public:
    XsAudio();
    ~XsAudio();
};


#endif //MYMUSIC_XSAUDIO_H
