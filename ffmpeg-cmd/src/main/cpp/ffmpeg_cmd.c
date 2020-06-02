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
Java_com_jdpxiaoming_ffmpeg_1cmd_FFmpegCmd_exec(JNIEnv *env, jclass clazz, jint cmdnum, jobjectArray cmdline) {
    // DO: implement exec()
    (*env)->GetJavaVM(env, &jvm);
    m_clazz = (*env)->NewGlobalRef(env, clazz);
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
    //新建线程 执行ffmpeg 命令
    ffmpeg_thread_run_cmd(cmdnum, argv);
    //注册ffmpeg命令执行完毕时的回调
    ffmpeg_thread_callback(ffmpeg_callback);
    free(strr);
    return 0;
}

JNIEXPORT void JNICALL
Java_com_jdpxiaoming_ffmpeg_1cmd_FFmpegCmd_exit(JNIEnv *env, jclass clazz) {
    // DO: implement exit()
    (*env)->GetJavaVM(env, &jvm);
    m_clazz = (*env)->NewGlobalRef(env, clazz);
    ffmpeg_thread_cancel();
    //stop the download thread .
    //1. stop downloading looper .
    isDownloading = 0;
    //2. stop the pthread_t .
    void *ret=NULL;
    pthread_join(pid_dump , &ret);
}


//开启下载线程的具体执行，类似runnable.
void * startDownloadThread(void* args){
    downloadFile(inputPath , outputPath);
}

JNIEXPORT jint JNICALL
Java_com_jdpxiaoming_ffmpeg_1cmd_FFmpegCmd_dump_1stream(JNIEnv *env, jclass clazz, jstring input,
                                                        jstring output) {
    inputPath = (char *) (*env)->GetStringUTFChars(env, input, 0);
    outputPath = (char *) (*env)->GetStringUTFChars(env, output, 0);
    pthread_create(&pid_dump, NULL ,startDownloadThread, NULL);
}


int downloadFile(const char* input, const char* output){
    LOGE("download file input:%s ,%s", input , output);
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    const char *in_filename, *out_filename;
    int ret, i;
    int stream_index = 0;
    int * stream_mapping = NULL;
    int stream_mapping_size = 0;
    //开始解码tag
    isDownloading = 1;

    // get the input path .
    in_filename  = input;
    // get the output path .
    out_filename = output;


    //use avformat to open the input file .
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        goto end;
    }

    //get stream head info from the av_format_context: ifmt_ctx .
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        goto end;
    }

    //check the input foramt protocal ? rtsp/file/http/htts/rtmp? .
    //no , it is an log print to console .and so on .
    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    //init the output AvformatCotnext with out_filename.
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    // get he input streams count , video /audio .
    stream_mapping_size = ifmt_ctx->nb_streams;

    stream_mapping = av_mallocz_array(stream_mapping_size,
                                      sizeof(*stream_mapping));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    //init the AVOutputFormat by avformatcontext .
    ofmt = ofmt_ctx->oformat;


    //test audio or video stream . write the codecparinfo to outfile .
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *out_stream;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;

        //filter useless streams .
        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }


        stream_mapping[i] = stream_index++;

        //create an output stream . maybe fileStream.
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        //copy the codepar from inputStream to the target outputStream .
        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters\n");
            goto end;
        }
        out_stream->codecpar->codec_tag = 0;
    }

    //print output streawm codec  info to file or console.
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    //open output file failed .
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open output file '%s'", out_filename);
            goto end;
        }
    }

    //write file header ex:Mp4 head info .
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        goto end;
    }

    //open a looper .
    while (isDownloading) {
        AVStream *in_stream, *out_stream;

        //get AVPacket .
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;

        //get the input stream .
        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        //filter dity data stream.
        if (pkt.stream_index >= stream_mapping_size ||
            stream_mapping[pkt.stream_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }

        pkt.stream_index = stream_mapping[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        //log the stream info to console.
//        log_packet(ifmt_ctx, &pkt, "in");

        /* copy packet */
        //caculate the time base of dpts & dts ,duration and pos . if mp4 has pos .
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
//        log_packet(ofmt_ctx, &pkt, "out");

        //exec write frame work .
        //往输出流中写一个分包
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }

    //write the file end trailer .
    //写输出流（文件）的文件尾
    av_write_trailer(ofmt_ctx);

end:

    avformat_close_input(&ifmt_ctx);

    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    av_freep(&stream_mapping);

    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;
}