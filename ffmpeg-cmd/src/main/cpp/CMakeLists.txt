cmake_minimum_required(VERSION 3.4.1)

# 使用通配符收集源文件
# 注意：include 目录下的 .c 文件不应该被直接编译，它们是通过 #include 方式被包含的
file(GLOB FFMPEG_SOURCES 
    ${CMAKE_SOURCE_DIR}/ffmpeg/*.c
    ${CMAKE_SOURCE_DIR}/ffmpeg/graph/*.c
    ${CMAKE_SOURCE_DIR}/ffmpeg/resources/*.c
    ${CMAKE_SOURCE_DIR}/ffmpeg/textformat/*.c
)

# 排除被其他文件包含的源文件，避免重复定义
list(REMOVE_ITEM FFMPEG_SOURCES 
    ${CMAKE_SOURCE_DIR}/ffmpeg/textformat/tw_buffer.c
    ${CMAKE_SOURCE_DIR}/ffmpeg/ffmpeg_enc.c  # 被 ffmpeg_dec.c 包含
)

file(GLOB ROOT_SOURCES 
    ${CMAKE_SOURCE_DIR}/*.c
)

# 确保所有源文件都被包含（用于调试）
message(STATUS "FFMPEG_SOURCES: ${FFMPEG_SOURCES}")
# message(STATUS "ROOT_SOURCES: ${ROOT_SOURCES}")

add_library( # Sets the name of the library.
        ffmpeg-cmd
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        ${FFMPEG_SOURCES}
        ${ROOT_SOURCES}
        )
#加载动态库，作为本地库使用.
add_library(ffmpeg
        SHARED
        IMPORTED )
set_target_properties( ffmpeg
        PROPERTIES IMPORTED_LOCATION
        ../../../../libs/${CMAKE_ANDROID_ARCH_ABI}/libffmpeg.so )

set(my_lib_path ${CMAKE_SOURCE_DIR}/../../../libs/${CMAKE_ANDROID_ARCH_ABI})
#设置c++编译标记，代表当前项目使用c++编译.-std=c++11 -L${my_lib_path}
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ")

# 引入头文件.
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ffmpeg)
include_directories(${CMAKE_SOURCE_DIR}/include)

find_library(
              log-lib
              log )

#链接共享库到目标库libnative-lib中.
target_link_libraries(
                       ffmpeg-cmd
                       ffmpeg
#                        avfilter avformat avcodec avutil swresample swscale avdevice
#                        (All FFmpeg libraries are already included in libffmpeg.so)
                        android
                        z
                        OpenSLES
                       ${log-lib} )