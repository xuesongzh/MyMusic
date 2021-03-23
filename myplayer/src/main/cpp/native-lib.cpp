#include <jni.h>
#include <string>
#include "XsFFmpeg.h"


extern "C"
{
#include <libavformat/avformat.h>
}

JavaVM *javaVM = NULL;
XsFFmpeg *ffmpeg = NULL;
XsCallJava *callJava = NULL;
XsPlaystatus *playStatus = NULL;
pthread_t startThread;

bool stopped = false;//防止多次release

//获取JVM对象，JVM是进程相关的，可以通过JVM来获取当前线程的JNIEnv，实现C++子线程调用Java方法
extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *jvm, void *reserved) {

    JNIEnv *env;
    javaVM = jvm;
    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zxsong_media_myplayer_player_XsPlayer_n_1prepared(JNIEnv *env, jobject instance,
                                                           jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);

    if (ffmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new XsCallJava(javaVM, env, instance);
        }
        callJava->onCallLoad(MAIN_THREAD, true);
        playStatus = new XsPlaystatus();
        ffmpeg = new XsFFmpeg(playStatus, callJava, source);
        ffmpeg->parpared();
    }
}

void *startCallback(void *data) {
    XsFFmpeg *fFmpeg = (XsFFmpeg *) data;
    ffmpeg->start();
    pthread_exit(&startThread);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zxsong_media_myplayer_player_XsPlayer_n_1start(JNIEnv *env, jobject instance) {

    if (ffmpeg != NULL) {
        pthread_create(&startThread, NULL, startCallback, ffmpeg);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zxsong_media_myplayer_player_XsPlayer_n_1pause(JNIEnv *env, jobject instance) {

    if (ffmpeg != NULL) {
        ffmpeg->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zxsong_media_myplayer_player_XsPlayer_n_1resume(JNIEnv *env, jobject instance) {

    if (ffmpeg != NULL) {
        ffmpeg->resume();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zxsong_media_myplayer_player_XsPlayer_n_1stop(JNIEnv *env, jobject instance) {

    if (stopped) {
        return;
    }

    jclass clz = env->GetObjectClass(instance);
    jmethodID jmid_next = env->GetMethodID(clz, "onCallNext", "()V");

    stopped = true;
    if (ffmpeg != NULL) {
        ffmpeg->release();
        delete ffmpeg;
        ffmpeg = NULL;
    }

    if (callJava != NULL) {
        delete callJava;
        callJava = NULL;
    }

    if (playStatus != NULL) {
        delete playStatus;
        playStatus = NULL;
    }
    stopped = false;

    env->CallVoidMethod(instance, jmid_next);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zxsong_media_myplayer_player_XsPlayer_n_1seek(JNIEnv *env, jobject instance,
                                                       jint seconds) {
    if (ffmpeg != NULL) {
        ffmpeg->seek(seconds);
    }

}