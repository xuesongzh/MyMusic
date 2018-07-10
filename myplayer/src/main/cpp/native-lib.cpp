#include <jni.h>
#include <string>

extern "C"
JNIEXPORT jstring
JNICALL
Java_com_zxsong_media_myplayer_Demo_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}