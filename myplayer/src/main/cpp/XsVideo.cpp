//
// Created by zxsong on 2021/3/22.
//

#include "XsVideo.h"

XsVideo::XsVideo(XsPlaystatus *playStatus, XsCallJava *callJava) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    queue = new XsQueue(playStatus);
}

XsVideo::~XsVideo() {

}

void *playVideo(void *data) {
    XsVideo *video = static_cast<XsVideo *>(data);

    while (video->playStatus != NULL && !video->playStatus->exit) {
        if (video->playStatus->seek) {
            av_usleep(1000 * 100);
            continue;
        }
//        if (video->playStatus->pause) {
//            av_usleep(1000 * 100);
//            continue;
//        }

        if (video->queue->getQueueSize() == 0) {
            if (!video->playStatus->load) {
                video->playStatus->load = true;
                video->callJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000 * 100);
            continue;
        } else {
            if (video->playStatus->load) {
                video->playStatus->load = false;
                video->callJava->onCallLoad(CHILD_THREAD, false);
            }
        }

        AVPacket *avPacket = av_packet_alloc();
        if (video->queue->getAvpacket(avPacket) != 0) {
            video->freePacket(avPacket);
            continue;
        }

        if (avcodec_send_packet(video->avCodecCtx, avPacket) != 0) {
            video->freePacket(avPacket);
            continue;
        }

        AVFrame *avFrame = av_frame_alloc();
        if (avcodec_receive_frame(video->avCodecCtx, avFrame) != 0) {
            video->freeFrame(avFrame);
            video->freePacket(avPacket);
            continue;
        }
        LOGE("子线程解码一个AVframe成功");
        video->freeFrame(avFrame);
        video->freePacket(avPacket);
    }
    return 0;
}

void XsVideo::play() {
    if (playStatus != NULL && !playStatus->exit) {
        pthread_create(&thread_play, NULL, playVideo, this);
    }
}

void XsVideo::freePacket(AVPacket *avPacket) {
    av_packet_free(&avPacket);
    av_free(avPacket);
    avPacket = NULL;
}

void XsVideo::freeFrame(AVFrame *avFrame) {
    av_frame_free(&avFrame);
    av_free(avFrame);
    avFrame = NULL;
}

void XsVideo::release() {
    pthread_join(thread_play, NULL);

    if (queue != NULL) {
        delete queue;
        queue = NULL;
    }

    if (avCodecCtx != NULL) {
        avcodec_close(avCodecCtx);
        avcodec_free_context(&avCodecCtx);
        avCodecCtx = NULL;
    }

    if (playStatus != NULL) {
        playStatus = NULL;
    }

    if (callJava != NULL) {
        callJava = NULL;
    }
}
