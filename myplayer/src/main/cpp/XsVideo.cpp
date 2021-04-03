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
        if (avFrame->format == AV_PIX_FMT_YUV420P) {
            LOGE("当前视频是YUV420P格式");
            video->callJava->onCallRenderYUV(video->avCodecCtx->width, video->avCodecCtx->height,
                                             avFrame->data[0], avFrame->data[1],
                                             avFrame->data[2]);

        } else {
            LOGE("当前视频不是YUV420P格式");
            //格式转换
            AVFrame *pFrameYUV420P = av_frame_alloc();
            int num = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, video->avCodecCtx->width,
                                               video->avCodecCtx->height, 1);
            uint8_t *buffer = static_cast<uint8_t *>(av_malloc(num * sizeof(uint8_t)));
            av_image_fill_arrays(pFrameYUV420P->data, pFrameYUV420P->linesize, buffer,
                                 AV_PIX_FMT_YUV420P, video->avCodecCtx->width,
                                 video->avCodecCtx->height, 1);
            SwsContext *sws_ctx = sws_getContext(video->avCodecCtx->width,
                                                 video->avCodecCtx->height,
                                                 video->avCodecCtx->pix_fmt,
                                                 video->avCodecCtx->width,
                                                 video->avCodecCtx->height,
                                                 AV_PIX_FMT_YUV420P,
                                                 SWS_BICUBIC,
                                                 NULL, NULL, NULL);

            if (!sws_ctx) {
                video->freeFrame(pFrameYUV420P);
                av_free(buffer);
//                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }
            sws_scale(sws_ctx, avFrame->data, avFrame->linesize, 0, avFrame->height,
                      pFrameYUV420P->data, pFrameYUV420P->linesize);
            //渲染

//            double diff = video->getFrameDiffTime(avFrame, NULL);
//            LOGE("diff is %f", diff);
//
//            av_usleep(video->getDelayTime(diff) * 1000000);

            video->callJava->onCallRenderYUV(video->avCodecCtx->width, video->avCodecCtx->height,
                                             pFrameYUV420P->data[0], pFrameYUV420P->data[1],
                                             pFrameYUV420P->data[2]);
        }
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

double XsVideo::getFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket) {
    return 0;
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

