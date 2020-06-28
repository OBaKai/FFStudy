
cmake_minimum_required(VERSION 3.4.1)

file(GLOB MY_CPP_SOURCE ${CMAKE_SOURCE_DIR}/src/main/cpp/*.cpp)
add_library( # Sets the name of the library.
        ff
        SHARED
        ${MY_CPP_SOURCE})

# 第三方库位置
set(ffmpeg_so_dir ${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI})

# 将所有库设置到系统环境变量
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${ffmpeg_so_dir}")

#add_library(avcodec SHARED IMPORTED)
#set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION
#        ${ffmpeg_so_dir}/libavcodec.so)
#
#add_library(avfilter SHARED IMPORTED)
#set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION
#        ${ffmpeg_so_dir}/libavfilter.so)
#
#add_library(avcodec SHARED IMPORTED)
#set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION
#        ${ffmpeg_so_dir}/libavcodec.so)
#
#add_library(avformat SHARED IMPORTED)
#set_target_properties(avformat PROPERTIES IMPORTED_LOCATION
#        ${ffmpeg_so_dir}/libavformat.so)
#
#add_library(avutil SHARED IMPORTED)
#set_target_properties(avutil PROPERTIES IMPORTED_LOCATION
#        ${ffmpeg_so_dir}/libavutil.so)
#
#add_library(swresample SHARED IMPORTED)
#set_target_properties(swresample PROPERTIES IMPORTED_LOCATION
#        ${ffmpeg_so_dir}/libswresample.so)
#
#add_library(swscale SHARED IMPORTED)
#set_target_properties(swscale PROPERTIES IMPORTED_LOCATION
#        ${ffmpeg_so_dir}/libswscale.so)

# 导入第三方头文件
include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/include)

find_library( # Sets the name of the path variable.
        log-lib
        log)

target_link_libraries( # Specifies the target library.
        ff
        ${log-lib}
        android
#        z
#        c
#        OpenSLES

        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale
        )