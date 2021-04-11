//
// Created by zxsong on 2018/9/18.
//

#ifndef MYMUSIC_XSCALLJAVA_H
#define MYMUSIC_XSCALLJAVA_H


#include <jni.h>
#include "AndroidLog.h"

#define MAIN_THREAD 0
#define CHILD_THREAD 1

class XsCallJava {

public:
    JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    jmethodID jmid_prepared;
    jmethodID jmid_load;
    jmethodID jmid_timeinfo;
    jmethodID jmid_error;
    jmethodID jmid_complete;
    jmethodID jmid_renderyuv;
    jmethodID jmid_supportmediacodec;
    jmethodID jmid_initmediacodec;
    jmethodID jmid_decodeavpacket;


public:
    XsCallJava(JavaVM *javaVM, JNIEnv *env, jobject obj);

    ~XsCallJava();

    void onCallPrepared(int type);

    void onCallLoad(int type, bool load);

    void onCallTimeInfo(int type, int cur, int total);

    void onCallError(int type, int code, char *msg);

    void onCallComplete(int type);

    void onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv);

    bool onCallIsSupportMediaCodec(const char *codec_name);

    void onCallInitMediaCodec(const char *mime, int width, int height, int csd0_size, int csd1_size,
                              uint8_t *csd_0, uint8_t *csd_1);

    void onCallDecodeAVPacket(uint8_t *packet_data, int size);

};


#endif //MYMUSIC_XSCALLJAVA_H
