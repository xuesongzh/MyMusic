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

public:
    XsCallJava(JavaVM *javaVM, JNIEnv *env, jobject obj);
    ~XsCallJava();

    void onCallPrepared(int type);

    void onCallLoad(int type, bool load);

    void onCallTimeInfo(int type, int cur, int total);

    void onCallError(int type, int code, char *msg);
};


#endif //MYMUSIC_XSCALLJAVA_H
