//
// Created by zxsong on 2018/9/18.
//

#include "XsCallJava.h"

/**
 * C++主线程调用Java方法
 * 1、根据jobject获取jclass（静态方法就不用这一步了）如：jclass clz = env->GetObjectClass(jobj);
 * 2、获取jmethodid 如： jmethodid jmid = env->GetMethodID(clz, "onError", "(ILjava/lang/String;)V")；
 * 3、调用方法 如： jenv->CallVoidMethod(jobj, jmid, code, jmsg); 
 *
 * C++子线程调用Java方法
 * 由于JNIEnv是线程相关的，所以子线程中不能使用创建线程的JNIEnv；
 * 而JVM是进程相关的，所以可以通过JVM来获取当前线程的JNIEnv，然后就可以调用Java的方法了
 * 1、获取JVM对象： JNI_OnLoad(JavaVM* vm,void* reserved)
 * 2、通过JVM获取JNIEnv：JNIEnv *env;
 * jvm->AttachCurrentThread(&env, 0);
 * jvm->DetachCurrentThread();
 */


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
    jmid_error = jniEnv->GetMethodID(clz, "onCallError", "(ILjava/lang/String;)V");
    jmid_complete = jniEnv->GetMethodID(clz, "onCallComplete", "()V");
    jmid_renderyuv = jniEnv->GetMethodID(clz, "onCallRenderYUV", "(II[B[B[B)V");
    jmid_supportmediacodec = jniEnv->GetMethodID(clz, "onCallIsSupportMediaCodec",
                                                 "(Ljava/lang/String;)Z");
    jmid_initmediacodec = jniEnv->GetMethodID(clz, "initMediaCodec", "(Ljava/lang/String;II[B[B)V");
    jmid_decodeavpacket = jniEnv->GetMethodID(clz, "decodeAVPacket", "([BI)V");
}

XsCallJava::~XsCallJava() {

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

void XsCallJava::onCallError(int type, int code, char *msg) {
    if (type == MAIN_THREAD) {
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jniEnv wrong");
            }
            return;
        }
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();
    }
}

void XsCallJava::onCallComplete(int type) {
    if (type == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_complete);
    } else if (type == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jniEnv wrong");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_complete);
        javaVM->DetachCurrentThread();
    }
}

void XsCallJava::onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        if (LOG_DEBUG) {
            LOGE("call onCallRenderYUV error!");
        }
        return;
    }

    jbyteArray y = jniEnv->NewByteArray(width * height);
    jniEnv->SetByteArrayRegion(y, 0, width * height, reinterpret_cast<const jbyte *>(fy));

    jbyteArray u = jniEnv->NewByteArray(width * height / 4);
    jniEnv->SetByteArrayRegion(u, 0, width * height / 4, reinterpret_cast<const jbyte *>(fu));

    jbyteArray v = jniEnv->NewByteArray(width * height / 4);
    jniEnv->SetByteArrayRegion(v, 0, width * height / 4, reinterpret_cast<const jbyte *>(fv));

    jniEnv->CallVoidMethod(jobj, jmid_renderyuv, width, height, y, u, v);

    jniEnv->DeleteLocalRef(y);
    jniEnv->DeleteLocalRef(u);
    jniEnv->DeleteLocalRef(v);

    javaVM->DetachCurrentThread();
}

bool XsCallJava::onCallIsSupportMediaCodec(const char *codec_name) {
    bool support = false;
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        if (LOG_DEBUG) {
            LOGE("call onCallIsSupportMediaCodec error!");
        }
        return support;
    }

    jstring type = jniEnv->NewStringUTF(codec_name);
    support = jniEnv->CallBooleanMethod(jobj, jmid_supportmediacodec, type);
    jniEnv->DeleteLocalRef(type);
    javaVM->DetachCurrentThread();
    return support;
}

void XsCallJava::onCallInitMediaCodec(const char *mime, int width, int height, int csd0_size,
                                      int csd1_size, uint8_t *csd_0, uint8_t *csd_1) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        if (LOG_DEBUG) {
            LOGE("call onCallInitMediaCodec worng");
        }
    }

    jstring type = jniEnv->NewStringUTF(mime);
    jbyteArray csd0 = jniEnv->NewByteArray(csd0_size);
    jniEnv->SetByteArrayRegion(csd0, 0, csd0_size, reinterpret_cast<const jbyte *>(csd_0));
    jbyteArray csd1 = jniEnv->NewByteArray(csd1_size);
    jniEnv->SetByteArrayRegion(csd1, 0, csd1_size, reinterpret_cast<const jbyte *>(csd_1));

    jniEnv->CallVoidMethod(jobj, jmid_initmediacodec, type, width, height, csd0, csd1);

    jniEnv->DeleteLocalRef(csd0);
    jniEnv->DeleteLocalRef(csd1);
    jniEnv->DeleteLocalRef(type);
    javaVM->DetachCurrentThread();

}

void XsCallJava::onCallDecodeAVPacket(uint8_t *packet_data, int size) {
    JNIEnv *jniEnv;
    if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
        if (LOG_DEBUG) {
            LOGE("call onCallDecodeAVPacket worng");
        }
    }

    jbyteArray data = jniEnv->NewByteArray(size);
    jniEnv->SetByteArrayRegion(data, 0, size, reinterpret_cast<const jbyte *>(packet_data));
    jniEnv->CallVoidMethod(jobj, jmid_decodeavpacket, data, size);
    jniEnv->DeleteLocalRef(data);
    javaVM->DetachCurrentThread();
}
