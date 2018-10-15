//
// Created by zxsong on 2018/9/24.
//

#ifndef MYMUSIC_XSQUEUE_H
#define MYMUSIC_XSQUEUE_H

#include "queue"
#include "pthread.h"
#include "AndroidLog.h"
#include "XsPlaystatus.h"

extern "C"
{
#include <libavcodec/avcodec.h>
};


class XsQueue {

public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    XsPlaystatus *playstatus = NULL;

public:
    XsQueue(XsPlaystatus *playstatus);
    ~XsQueue();

    int putAvpacket(AVPacket *packet);
    int getAvpacket(AVPacket *packet);

    int getQueueSize();

    void clearAvpacket();
};


#endif //MYMUSIC_XSQUEUE_H
