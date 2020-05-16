#include "ffmpeg_cmd.h"

#include <jni.h>
#include <string.h>
#include "ffmpeg_thread.h"
#include "android_log.h"
#include "cmdutils.h"

static JavaVM *jvm = NULL;
//java虚拟机
static jclass m_clazz = NULL;//当前类(面向java)

/**
 * 回调执行Java方法
 * 参看 Jni反射+Java反射
 */
void callJavaMethod(JNIEnv *env, jclass clazz,int ret) {
    if (clazz == NULL) {
        LOGE("---------------clazz isNULL---------------");
        return;
    }
    //获取方法ID (I)V指的是方法签名 通过javap -s -public FFmpegCmd 命令生成
    jmethodID methodID = (*env)->GetStaticMethodID(env, clazz, "onExecuted", "(I)V");
    if (methodID == NULL) {
        LOGE("---------------methodID isNULL---------------");
        return;
    }
    //调用该java方法
    (*env)->CallStaticVoidMethod(env, clazz, methodID,ret);
}
void callJavaMethodProgress(JNIEnv *env, jclass clazz,float ret) {
    if (clazz == NULL) {
        LOGE("---------------clazz isNULL---------------");
        return;
    }
    //获取方法ID (I)V指的是方法签名 通过javap -s -public FFmpegCmd 命令生成
    jmethodID methodID = (*env)->GetStaticMethodID(env, clazz, "onProgress", "(F)V");
    if (methodID == NULL) {
        LOGE("---------------methodID isNULL---------------");
        return;
    }
    //调用该java方法
    (*env)->CallStaticVoidMethod(env, clazz, methodID,ret);
}

void callJavaMethodComplete(JNIEnv *env, jclass clazz) {
    if (clazz == NULL) {
        LOGE("---------------clazz isNULL---------------");
        return;
    }
    //获取方法ID (I)V指的是方法签名 通过javap -s -public FFmpegCmd 命令生成
    jmethodID methodID = (*env)->GetStaticMethodID(env, clazz, "onComplete", "()V");
    if (methodID == NULL) {
        LOGE("---------------methodID isNULL---------------");
        return;
    }
    //调用该java方法
    (*env)->CallStaticVoidMethod(env, clazz, methodID);
}

/**
 * c语言-线程回调
 */
static void ffmpeg_callback(int ret) {
    JNIEnv *env;
    //附加到当前线程从JVM中取出JNIEnv, C/C++从子线程中直接回到Java里的方法时  必须经过这个步骤
    (*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL);
    callJavaMethod(env, m_clazz,ret);

    //完毕-脱离当前线程
    (*jvm)->DetachCurrentThread(jvm);
}

/**
 * 时间进度回调.
 * @param progress
 */
void ffmpeg_progress(float progress) {
    JNIEnv *env;
    (*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL);
    callJavaMethodProgress(env, m_clazz,progress);
    (*jvm)->DetachCurrentThread(jvm);
}

/**
 * 正常结束任务.
 * @param errorCode
 */
void ffmpeg_complete(int errorCode){
    JNIEnv *env;
    (*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL);
    callJavaMethodComplete(env, m_clazz);
    (*jvm)->DetachCurrentThread(jvm);
}


JNIEXPORT jint JNICALL
Java_com_example_ffmpegtools_MainActivity_exec(JNIEnv *env, jclass clazz, jint cmdnum, jobjectArray cmdline)
{
    (*env)->GetJavaVM(env, &jvm);
    m_clazz = (*env)->NewGlobalRef(env, clazz);
    //---------------------------------C语言 反射Java 相关----------------------------------------
    //---------------------------------java 数组转C语言数组----------------------------------------
    int i = 0;//满足NDK所需的C99标准
    char **argv = NULL;//命令集 二维指针
    jstring *strr = NULL;

    if (cmdline != NULL) {
        argv = (char **) malloc(sizeof(char *) * cmdnum);
        strr = (jstring *) malloc(sizeof(jstring) * cmdnum);

        for (i = 0; i < cmdnum; ++i) {//转换
            strr[i] = (jstring)(*env)->GetObjectArrayElement(env, cmdline, i);
            argv[i] = (char *) (*env)->GetStringUTFChars(env, strr[i], 0);
        }

    }
    //---------------------------------java 数组转C语言数组----------------------------------------
    //---------------------------------执行FFmpeg命令相关----------------------------------------
    //新建线程 执行ffmpeg 命令
    ffmpeg_thread_run_cmd(cmdnum, argv);
    //注册ffmpeg命令执行完毕时的回调
    ffmpeg_thread_callback(ffmpeg_callback);
    free(strr);
    return 0;

    //test code  by poe.cai 2020/05/16 直接代码测试会结束退出.
    /*char *argv[cmdnum];

    int i;
    for (i = 0; i < cmdnum; i++) {
        jstring js = (jstring) (*env)->GetObjectArrayElement(env, cmdline, i);
        argv[i] = (char *) (*env)->GetStringUTFChars(env, js, 0);
    }
    return ffmpeg_exec(cmdnum, argv);*/

}

JNIEXPORT void JNICALL
Java_com_example_ffmpegtools_MainActivity_exit(JNIEnv *env, jclass type)
{
    (*env)->GetJavaVM(env, &jvm);
    m_clazz = (*env)->NewGlobalRef(env, type);
    ffmpeg_thread_cancel();
}

JNIEXPORT jint JNICALL
Java_com_example_ffmpegtools_FFmpegCmd_exec(JNIEnv *env, jclass clazz, jint cmdnum, jobjectArray cmdline) {
    // DO: implement exec()
    (*env)->GetJavaVM(env, &jvm);
    m_clazz = (*env)->NewGlobalRef(env, clazz);
    //---------------------------------C语言 反射Java 相关----------------------------------------
    //---------------------------------java 数组转C语言数组----------------------------------------
    int i = 0;//满足NDK所需的C99标准
    char **argv = NULL;//命令集 二维指针
    jstring *strr = NULL;

    if (cmdline != NULL) {
        argv = (char **) malloc(sizeof(char *) * cmdnum);
        strr = (jstring *) malloc(sizeof(jstring) * cmdnum);

        for (i = 0; i < cmdnum; ++i) {//转换
            strr[i] = (jstring)(*env)->GetObjectArrayElement(env, cmdline, i);
            argv[i] = (char *) (*env)->GetStringUTFChars(env, strr[i], 0);
        }

    }
    //---------------------------------java 数组转C语言数组----------------------------------------
    //---------------------------------执行FFmpeg命令相关----------------------------------------
    //新建线程 执行ffmpeg 命令
    ffmpeg_thread_run_cmd(cmdnum, argv);
    //注册ffmpeg命令执行完毕时的回调
    ffmpeg_thread_callback(ffmpeg_callback);
    free(strr);
    return 0;
}

JNIEXPORT void JNICALL
Java_com_example_ffmpegtools_FFmpegCmd_exit(JNIEnv *env, jclass clazz) {
    // DO: implement exit()
    (*env)->GetJavaVM(env, &jvm);
    m_clazz = (*env)->NewGlobalRef(env, clazz);
    ffmpeg_thread_cancel();
}