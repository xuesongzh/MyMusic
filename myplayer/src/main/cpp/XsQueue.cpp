//
// Created by zxsong on 2018/9/24.
//

#include "XsQueue.h"

XsQueue::XsQueue(XsPlaystatus *playstatus) {
    this->playstatus = playstatus;
    pthread_mutex_init(&mutexPacket, NULL);
    pthread_cond_init(&condPacket, NULL);
}

XsQueue::~XsQueue() {


}

int XsQueue::putAvpacket(AVPacket *packet) {
    //加锁
    pthread_mutex_lock(&mutexPacket);
    //入队
    queuePacket.push(packet);
    if(LOG_DEBUG)
    {
        LOGD("放入一个AVpacket到队列里面，个数为：%d", queuePacket.size());
    }
    //发送消息给消费者
    pthread_cond_signal(&condPacket);
    //解锁
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int XsQueue::getAvpacket(AVPacket *packet) {
    pthread_mutex_lock(&mutexPacket);

    while (playstatus != NULL && !playstatus->exit) {
        if (queuePacket.size() > 0) {
            AVPacket *avPacket = queuePacket.front();
            //把avPacket的内存数据拷贝到packet内存中
            if (av_packet_ref(packet,avPacket) == 0) {
                queuePacket.pop();
            }
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            if(LOG_DEBUG) {
                LOGD("从队列里面取出一个AVpacket，还剩下 %d 个", queuePacket.size());
            }
            break;
        } else {
            pthread_cond_wait(&condPacket, &mutexPacket);
        }
    }

    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int XsQueue::getQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutexPacket);
    size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;
}
