#include <jni.h>
#include <string>
#include "XsFFmpeg.h"


extern "C"
{
#include <libavformat/avformat.h>
}

JavaVM *javaVM = NULL;
XsFFmpeg *fFmpeg = NULL;
XsCallJava *callJava = NULL;
XsPlaystatus *playstatus = NULL;

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {

    JNIEnv *env;
    javaVM = jvm;
    if(jvm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zxsong_media_myplayer_player_XsPlayer_n_1prepared(JNIEnv *env, jobject instance,
                                                           jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);

    if (fFmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new XsCallJava(javaVM, env, instance);
        }
        playstatus = new XsPlaystatus();
        fFmpeg = new XsFFmpeg(playstatus, callJava, source);
        fFmpeg->parpared();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zxsong_media_myplayer_player_XsPlayer_n_1start(JNIEnv *env, jobject instance) {

    if (fFmpeg != NULL) {
        fFmpeg->start();
    }
}