//
// Created by zxsong on 2018/9/24.
//

#include "XsQueue.h"

/***  生产者消费者模型
//线程锁对象
pthread_mutex_t mutex;

//用于初始化pthread_mutex_t锁对象
pthread_mutex_init(&mutex, NULL);

//用于销毁pthread_mutex_t锁对象
pthread_mutex_destroy(&mutex)

//线程条件对象
pthread_cond_t cond;
//用于初始化pthread_cond_t线程条件对象
pthread_cond_init(&cond, NULL);
//用于销毁pthread_cond_t线程条件对象
pthread_cond_destroy(&cond);

//用于上锁mutex,本线程上锁后的其他变量是不能被别的线程操作
pthread_mutex_lock(&mutex);

//用于解锁mutex，解锁后的其他变量可以被其他线程操作
pthread_mutex_unlock(&mutex);

//用于发出条件信号
 pthread_cond_signal(&cond);

//用于线程阻塞等待，直到pthread_cond_signal发出条件信号后才退出线程阻塞执行后面的操作
pthread_cond_wait(&cond, &mutex);
 */

XsQueue::XsQueue(XsPlaystatus *playstatus) {
    this->playstatus = playstatus;
    pthread_mutex_init(&mutexPacket, NULL);
    pthread_cond_init(&condPacket, NULL);
}

XsQueue::~XsQueue() {
    clearAvpacket();
    pthread_mutex_destroy(&mutexPacket);
    pthread_cond_destroy(&condPacket);
}

int XsQueue::putAvpacket(AVPacket *packet) {
    //加锁
    pthread_mutex_lock(&mutexPacket);
    //入队
    queuePacket.push(packet);
    /*if(LOG_DEBUG) {
        LOGD("放入一个AVpacket到队列里面，个数为：%d", queuePacket.size());
    }*/
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
            /*if(LOG_DEBUG) {
                LOGD("从队列里面取出一个AVpacket，还剩下 %d 个", queuePacket.size());
            }*/
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

void XsQueue::clearAvpacket() {
    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);

    while (!queuePacket.empty()) {
        AVPacket *packet = queuePacket.front();
        queuePacket.pop();
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    pthread_mutex_unlock(&mutexPacket);
}

void XsQueue::notifyQueue() {
    pthread_cond_signal(&condPacket);
}


