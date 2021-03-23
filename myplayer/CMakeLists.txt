
cmake_minimum_required(VERSION 3.4.1)

# 导入include路径
include_directories(src/main/cpp/include)

add_library( # Sets the name of the library.
        native-lib

        SHARED

        src/main/cpp/XsQueue.cpp
        src/main/cpp/XsPlaystatus.cpp
        src/main/cpp/XsAudio.cpp
        src/main/cpp/XsVideo.cpp
        src/main/cpp/XsCallJava.cpp
        src/main/cpp/XsFFmpeg.cpp
        src/main/cpp/native-lib.cpp)
# 添加动态库
add_library(avcodec SHARED IMPORTED)
# 设置动态库路径
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/libs/${ANDROID_ABI}/libavcodec.so)


add_library(avdevice SHARED IMPORTED)
set_target_properties(avdevice
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/libs/${ANDROID_ABI}/libavdevice.so)


add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/libs/${ANDROID_ABI}/libavfilter.so)


add_library(avformat SHARED IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/libs/${ANDROID_ABI}/libavformat.so)


add_library(avutil SHARED IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/libs/${ANDROID_ABI}/libavutil.so)

add_library(postproc SHARED IMPORTED)
set_target_properties(postproc
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/libs/${ANDROID_ABI}/libpostproc.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/libs/${ANDROID_ABI}/libswresample.so)


add_library(swscale SHARED IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/src/main/libs/${ANDROID_ABI}/libswscale.so)

# 链接动态库
target_link_libraries( # Specifies the target library.
        native-lib

        avcodec
        avdevice
        avfilter
        avformat
        avutil
        postproc
        swresample
        swscale

        OpenSLES

        # Links the target library to the log library
        # included in the NDK.
        log)