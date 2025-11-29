cmake_minimum_required(VERSION 3.4.1)

# 使用通配符收集源文件
file(GLOB FFMPEG_SOURCES 
    ${CMAKE_SOURCE_DIR}/ffmpeg/*.c
    ${CMAKE_SOURCE_DIR}/ffmpeg/graph/*.c
    ${CMAKE_SOURCE_DIR}/ffmpeg/resources/*.c
    ${CMAKE_SOURCE_DIR}/ffmpeg/textformat/*.c
    ${CMAKE_SOURCE_DIR}/include/libavdevice/*.c
)
file(GLOB ROOT_SOURCES 
    ${CMAKE_SOURCE_DIR}/*.c
)

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
#                        avfilter avformat avcodec avutil swresample swscale
                        android
                        z
                        OpenSLES
                       ${log-lib} )