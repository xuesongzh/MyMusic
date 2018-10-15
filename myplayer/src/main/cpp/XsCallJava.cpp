//
// Created by zxsong on 2018/9/18.
//

#include "XsCallJava.h"

XsCallJava::XsCallJava(JavaVM *javaVM, JNIEnv *env, jobject obj) {

    this->javaVM = javaVM;
    this->jniEnv = env;
    this->jobj = env->NewGlobalRef(obj);//全局引用的方式在多线程间使用

    jclass clz = jniEnv->GetObjectClass(jobj);
    if (!clz) {
        if (LOG_DEBUG) {
            LOGE("get jclass wrong");
        }
        return;
    }

    jmid_prepared = jniEnv->GetMethodID(clz, "onCallPrepared", "()V");
    jmid_load = jniEnv->GetMethodID(clz, "onCallLoad", "(Z)V");
    jmid_timeinfo = jniEnv->GetMethodID(clz, "onCallTimeInfo", "(II)V");
}

void XsCallJava::onCallPrepared(int type) {

    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_prepared);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jniEnv wrong");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_prepared);
        javaVM->DetachCurrentThread();
    }
}

void XsCallJava::onCallLoad(int type, bool load) {

    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_load, load);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jniEnv wrong");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_load, load);
        javaVM->DetachCurrentThread();
    }
}

void XsCallJava::onCallTimeInfo(int type, int cur, int total) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, cur, total);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jniEnv wrong");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, cur, total);
        javaVM->DetachCurrentThread();
    }
}

XsCallJava::~XsCallJava() {

}
