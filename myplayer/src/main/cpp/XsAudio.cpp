//
// Created by zxsong on 2018/9/19.
//

#include "XsAudio.h"

XsAudio::XsAudio(XsPlaystatus *playstatus) {
    this->playstatus = playstatus;
    queue = new XsQueue(playstatus);
    buffer = (uint8_t *) av_malloc(44100 * 2 * 2);//1s的PCM大小
}

XsAudio::~XsAudio() {

}

void *decodePlay(void *data) {
    XsAudio *xsAudio = (XsAudio *) data;
    xsAudio->resampleAudio();
    pthread_exit(&xsAudio->playThread);
}

void XsAudio::play() {
    pthread_create(&playThread, NULL, decodePlay, this);
}

FILE *outFile = fopen("/storage/emulated/0/mydream.pcm", "w");

//重采样生成PCM数据
int XsAudio::resampleAudio() {

    while (playstatus != NULL && !playstatus->exit) {
        avPacket = av_packet_alloc();
        if (queue->getAvpacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        ret = avcodec_send_packet(avCodecCtx, avPacket);
        if (ret != 0) {//error
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }

        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecCtx, avFrame);
        if (ret == 0) {
            //判断声道布局和声道数，异常情况
            if (avFrame->channels > 0 && avFrame->channel_layout == 0) {
                avFrame->channel_layout = (uint64_t) av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swrContext;
            swrContext = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,// 输出声道布局
                    AV_SAMPLE_FMT_S16,//输出采样位数格式
                    avFrame->sample_rate,//输出采样率
                    avFrame->channel_layout,// 输入声道布局
                    (AVSampleFormat) avFrame->format,//输入采样位数格式
                    avFrame->sample_rate,//输入采样率
                    NULL,
                    NULL);
            if (swrContext == NULL || swr_init(swrContext) < 0) {//failed
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                if (swrContext != NULL) {
                    swr_free(&swrContext);
                    swrContext = NULL;
                }
                continue;
            }

            int nb = swr_convert(
                    swrContext,
                    &buffer,//转码后输出的PCM数据大小
                    avFrame->nb_samples,//输出采样个数
                    (const uint8_t **) avFrame->data,//原始压缩数据
                    avFrame->nb_samples);//输入采样个数

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            LOGD("每帧的大小：%d",data_size);
            fwrite(buffer, 1, data_size, outFile);

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swrContext);
            swrContext = NULL;
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }

    }
    fclose(outFile);
    return data_size;
}


