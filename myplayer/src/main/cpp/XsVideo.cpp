//
// Created by zxsong on 2021/3/22.
//

#include "XsVideo.h"

XsVideo::XsVideo(XsPlaystatus *playStatus, XsCallJava *callJava) {
    this->playStatus = playStatus;
    this->callJava = callJava;
    queue = new XsQueue(playStatus);
    pthread_mutex_init(&codecMutex, NULL);
}

XsVideo::~XsVideo() {
    pthread_mutex_destroy(&codecMutex);
}

void *playVideo(void *data) {
    XsVideo *video = static_cast<XsVideo *>(data);

    while (video->playStatus != NULL && !video->playStatus->exit) {
        if (video->playStatus->seek) {
            av_usleep(1000 * 100);
            continue;
        }
        if (video->playStatus->pause) {
            av_usleep(1000 * 100);
            continue;
        }

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

        if (video->codecType == CODEC_MEDIA_CODEC) {
            // 􏰀􏰁􏰙􏰚􏰲􏰳􏰴添加解码头信息
            if (av_bsf_send_packet(video->bsfCtx, avPacket) != 0) {
                video->freePacket(avPacket);
                continue;
            }
            while (av_bsf_receive_packet(video->bsfCtx, avPacket) == 0) {
                LOGE("开始硬解码");

                double diff = video->getFrameDiffTime(NULL, avPacket);
                LOGE("diff is %f", diff);

                av_usleep(video->getDelayTime(diff) * 1000000);
                video->callJava->onCallDecodeAVPacket(avPacket->data, avPacket->size);

                av_packet_free(&avPacket);
                av_free(avPacket);
                continue;
            }
            avPacket = NULL;
        } else if (video->codecType == CODEC_YUV) {
            pthread_mutex_lock(&video->codecMutex);
            if (avcodec_send_packet(video->avCodecCtx, avPacket) != 0) {
                video->freePacket(avPacket);
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }

            AVFrame *avFrame = av_frame_alloc();
            if (avcodec_receive_frame(video->avCodecCtx, avFrame) != 0) {
                video->freeFrame(avFrame);
                video->freePacket(avPacket);
                pthread_mutex_unlock(&video->codecMutex);
                continue;
            }

            LOGE("子线程解码一个AVframe成功");

            double diff = video->getFrameDiffTime(avFrame, NULL);
            LOGE("diff is %f", diff);

            if (avFrame->format == AV_PIX_FMT_YUV420P) {
                LOGE("当前视频是YUV420P格式");
                // 视频同步音频
                av_usleep(video->getDelayTime(diff) * 1000000);
                video->callJava->onCallRenderYUV(avFrame->linesize[0],
                                                 video->avCodecCtx->height,
                                                 avFrame->data[0],
                                                 avFrame->data[1],
                                                 avFrame->data[2]);
                LOGE("width = %d, height = %d", video->avCodecCtx->width, video->avCodecCtx->height);
                LOGE("linesize = %d", avFrame->linesize[0]);
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
                    pthread_mutex_unlock(&video->codecMutex);
                    continue;
                }
                sws_scale(sws_ctx, avFrame->data, avFrame->linesize, 0, avFrame->height,
                          pFrameYUV420P->data, pFrameYUV420P->linesize);

                //渲染
                av_usleep(video->getDelayTime(diff) * 1000000);
                video->callJava->onCallRenderYUV(video->avCodecCtx->width,
                                                 video->avCodecCtx->height,
                                                 pFrameYUV420P->data[0],
                                                 pFrameYUV420P->data[1],
                                                 pFrameYUV420P->data[2]);
            }
            video->freeFrame(avFrame);
            video->freePacket(avPacket);
            pthread_mutex_unlock(&video->codecMutex);
        }
    }
    return nullptr;
}

void XsVideo::play() {
    if (playStatus != NULL && !playStatus->exit) {
        pthread_create(&thread_play, NULL, playVideo, this);
    }
}

double XsVideo::getFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket) {
    double pts = 0;
    if (avFrame != NULL) {
        pts = av_frame_get_best_effort_timestamp(avFrame);
    }
    if (avPacket != NULL) {
        pts = avPacket->pts;
    }
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }

    pts *= av_q2d(time_base);

    if (pts > 0) {
        playTime = pts;
    }

    double diff = audio->play_time - playTime;
    return diff;
}

double XsVideo::getDelayTime(double diff) {
    if (diff > 0.003) { //音频快了
        delayTime = delayTime * 2 / 3;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    } else if (diff < -0.003) { //音频慢了
        delayTime = delayTime * 3 / 2;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    }

    if (diff >= 0.5) {
        delayTime = 0;
    } else if (diff <= -0.5) {
        delayTime = defaultDelayTime * 2;
    }

    if (fabs(diff) >= 10) {
        delayTime = defaultDelayTime;
    }
    return delayTime;
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
    if (queue != NULL) {
        queue->notifyQueue();
    }
    pthread_join(thread_play, NULL);

    if (queue != NULL) {
        delete queue;
        queue = NULL;
    }

    if (bsfCtx != NULL) {
        av_bsf_free(&bsfCtx);
        bsfCtx = NULL;
    }

    if (avCodecCtx != NULL) {
        pthread_mutex_lock(&codecMutex);
        avcodec_close(avCodecCtx);
        avcodec_free_context(&avCodecCtx);
        avCodecCtx = NULL;
        pthread_mutex_unlock(&codecMutex);
    }

    if (playStatus != NULL) {
        playStatus = NULL;
    }

    if (callJava != NULL) {
        callJava = NULL;
    }
}


