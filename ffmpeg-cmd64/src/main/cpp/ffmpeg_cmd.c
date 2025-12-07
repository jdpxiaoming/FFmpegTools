#include "ffmpeg_cmd.h"

#include <jni.h>
#include <string.h>
#include <include/libavutil/audio_fifo.h>
#include <include/libavutil/opt.h>
#include <include/libavfilter/buffersrc.h>
#include <include/libavfilter/buffersink.h>
#include <include/libavutil/avassert.h>
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

void callJavaMethodFailure(JNIEnv *env, jclass clazz) {
    if (clazz == NULL) {
        LOGE("---------------clazz isNULL---------------");
        return;
    }
    //获取方法ID (I)V指的是方法签名 通过javap -s -public FFmpegCmd 命令生成
    jmethodID methodID = (*env)->GetStaticMethodID(env, clazz, "onFailure", "()V");
    if (methodID == NULL) {
        LOGE("---------------methodID isNULL---------------");
        return;
    }
    //调用该java方法
    (*env)->CallStaticVoidMethod(env, clazz, methodID);
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
 * 取消任务回调.
 * @param env
 * @param clazz
 */
void callJavaMethodCancelFinish(JNIEnv *env, jclass clazz) {
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
    //先取消本地线程.
    ffmpeg_thread_cancel();

    JNIEnv *env;
    (*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL);
    callJavaMethodComplete(env, m_clazz);
    (*jvm)->DetachCurrentThread(jvm);
}

/**
 * 取消下载任务.
 * @param errorCode
 */
void ffmpeg_cancel_finish(int errorCode){
    //先取消本地线程.
    ffmpeg_thread_cancel();

    JNIEnv *env;
    (*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL);
    callJavaMethodComplete(env, m_clazz);
    (*jvm)->DetachCurrentThread(jvm);
}

/**
 * 异常结束任务.
 * @param errorCode
 */
void ffmpeg_failure(int errorCode){
    //先取消本地线程.
    ffmpeg_thread_cancel();

    JNIEnv *env;
    (*jvm)->AttachCurrentThread(jvm, (void **) &env, NULL);
    callJavaMethodFailure(env, m_clazz);
    (*jvm)->DetachCurrentThread(jvm);
}


/**
 * 执行ffmpeg命令主要入口.
 * @param env
 * @param clazz
 * @param cmdnum 命令长度.
 * @param cmdline 命令集合.
 * @return
 */
JNIEXPORT jint JNICALL
Java_com_jdpxiaoming_ffmpeg_1cmd_FFmpegCmd_exec(JNIEnv *env, jclass clazz, jint cmdnum, jobjectArray cmdline) {
    LOGE("Java_com_jdpxiaoming_ffmpeg_1cmd_FFmpegCmd_exec execute!!!");
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
    if(pid_dump){
        pthread_join(pid_dump , &ret);
    }else{
        LOGE("pid_dump is NULl , do noting to exit this program !");
    }
    //通知回调stop结束.

}

//开启下载线程的具体执行，类似runnable.
void * startDownloadThread(void* args){
    downloadFile(inputPath , outputPath);
}

//开启下载线程的具体执行，类似runnable.
void * startRtspDownloadThread(void* args){
    downloadFileAAc(inputPath , outputPath);
}

JNIEXPORT jint JNICALL
Java_com_jdpxiaoming_ffmpeg_1cmd_FFmpegCmd_dump_1stream(JNIEnv *env, jclass clazz, jstring input,
                                                        jstring output) {
    LOGE("Java_com_jdpxiaoming_ffmpeg_1cmd_FFmpegCmd_dump_1stream()~");
    inputPath = (char *) (*env)->GetStringUTFChars(env, input, 0);
    outputPath = (char *) (*env)->GetStringUTFChars(env, output, 0);
    pthread_create(&pid_dump, NULL ,startDownloadThread, NULL);
}

/**
 * dowload file . remuxing to mp4.
 * @param input
 * @param output
 * @return
 */
int downloadFile(const char* input, const char* output){
    LOGE("========== downloadFile START ==========");
    LOGE("Input: %s", input);
    LOGE("Output: %s", output);
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    const char *in_filename, *out_filename;
    int ret, i;
    int stream_index = 0;
    int * stream_mapping = NULL;
    int stream_mapping_size = 0;
    int packet_count = 0;
    int64_t total_bytes_written = 0;
    //开始解码tag
    isDownloading = 1;

    // get the input path .
    in_filename  = input;
    // get the output path .
    out_filename = output;
    //设置读取参数
    AVDictionary* inputDic = NULL ;
//    memset(inputDic ,0, sizeof(inputDic));
    av_dict_set(&inputDic, "rtsp_transport", "tcp", 0);
    LOGE("RTSP transport set to TCP");


    //use avformat to open the input file .
    LOGE("Opening input file: %s", in_filename);
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, &inputDic)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        LOGE("ERROR: Failed to open input file '%s', error: %s (%d)", in_filename, av_err2str(ret), ret);
        goto end;
    }
    LOGE("Input file opened successfully");

    //get stream head info from the av_format_context: ifmt_ctx .
    LOGE("Finding stream information...");
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        LOGE("ERROR: Failed to retrieve input stream information, error: %s (%d)", av_err2str(ret), ret);
        goto end;
    }
    LOGE("Stream information found: %d streams", ifmt_ctx->nb_streams);

    //check the input foramt protocal ? rtsp/file/http/htts/rtmp? .
    //no , it is an log print to console .and so on .将流的一些配置信息保存在输入环境信息
    av_dump_format(ifmt_ctx, 0, in_filename, 0);
    LOGE("Input format: %s, duration: %lld", ifmt_ctx->iformat->name, ifmt_ctx->duration);

    //init the output AvformatCotnext with out_filename.据输出flv文件名称和路径得到输出环境信息
    LOGE("Creating output context for: %s", out_filename);
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        LOGE("ERROR: Could not create output context for '%s'", out_filename);
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    LOGE("Output context created, format: %s", ofmt_ctx->oformat->name);

    // get he input streams count , video /audio .
    stream_mapping_size = ifmt_ctx->nb_streams;

    //在FFmpeg 7.x及以后版本中，你应该使用av_mallocz和av_malloc_array的组合来替代av_mallocz_array
    //av_mallocz_array 已被弃用，因为它在分配内存后立即将其清零，这在某些情况下是不必要的，并且可能导致性能问题。
    stream_mapping = av_malloc_array(stream_mapping_size,
                                     sizeof(*stream_mapping));
    memset(stream_mapping , 0 , stream_mapping_size * sizeof(stream_mapping[0]));

    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    //init the AVOutputFormat by avformatcontext .
    ofmt = ofmt_ctx->oformat;


    //test audio or video stream . write the codecparinfo to outfile .
    LOGE("Processing %d input streams...", ifmt_ctx->nb_streams);
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *out_stream;
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;
        const char *codec_type_name = av_get_media_type_string(in_codecpar->codec_type);
        LOGE("Stream %d: type=%s, codec_id=%d", i, codec_type_name ? codec_type_name : "unknown", in_codecpar->codec_id);

        //filter useless streams .
        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            LOGE("Stream %d: skipped (not audio/video/subtitle)", i);
            continue;
        }


        stream_mapping[i] = stream_index++;
        LOGE("Stream %d: mapped to output stream %d", i, stream_mapping[i]);

        //create an output stream . maybe fileStream.
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            LOGE("ERROR: Failed allocating output stream %d", i);
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        //copy the codepar from inputStream to the target outputStream .
        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters\n");
            LOGE("ERROR: Failed to copy codec parameters for stream %d, error: %s (%d)", i, av_err2str(ret), ret);
            goto end;
        }
        out_stream->codecpar->codec_tag = 0;
        LOGE("Stream %d: codec parameters copied successfully", i);
    }
    LOGE("Total output streams created: %d", stream_index);

    //print output streawm codec  info to file or console.
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    //open output file failed .
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        LOGE("Opening output file: %s", out_filename);
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open output file '%s'", out_filename);
            LOGE("ERROR: Could not open output file '%s', error: %s (%d)", out_filename, av_err2str(ret), ret);
            goto end;
        }
        LOGE("Output file opened successfully");
    } else {
        LOGE("Output format does not require file I/O");
    }

    //write file header ex:Mp4 head info .
    LOGE("Writing output file header...");
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        LOGE("ERROR: Failed to write output file header, error: %s (%d)", av_err2str(ret), ret);
        goto end;
    }
    LOGE("Output file header written successfully");

    //open a looper .
    LOGE("Starting packet reading loop...");
    while (isDownloading) {
        AVStream *in_stream, *out_stream;

        //get AVPacket .
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                LOGE("End of input stream reached");
            } else {
                LOGE("ERROR: Failed to read frame, error: %s (%d)", av_err2str(ret), ret);
            }
            break;
        }

        packet_count++;
        if (packet_count % 100 == 0) {
            LOGE("Processed %d packets, total bytes: %lld", packet_count, total_bytes_written);
        }

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
            LOGE("ERROR: Failed to mux packet #%d, error: %s (%d)", packet_count, av_err2str(ret), ret);
            break;
        }
        total_bytes_written += pkt.size;
        av_packet_unref(&pkt);
    }

    LOGE("Packet reading loop ended. Total packets: %d, Total bytes: %lld", packet_count, total_bytes_written);

    //write the file end trailer .
    //写输出流（文件）的文件尾
    LOGE("Writing output file trailer...");
    ret = av_write_trailer(ofmt_ctx);
    if (ret < 0) {
        LOGE("ERROR: Failed to write trailer, error: %s (%d)", av_err2str(ret), ret);
    } else {
        LOGE("Output file trailer written successfully");
    }

    end:
    LOGE("========== downloadFile CLEANUP ==========");

    if (ifmt_ctx) {
        LOGE("Closing input context...");
        avformat_close_input(&ifmt_ctx);
    }

    /* close output */
    if (ofmt_ctx) {
        if (!(ofmt->flags & AVFMT_NOFILE)) {
            LOGE("Closing output file I/O...");
            avio_closep(&ofmt_ctx->pb);
        }
        LOGE("Freeing output context...");
        avformat_free_context(ofmt_ctx);
    }

    if (stream_mapping) {
        av_freep(&stream_mapping);
    }

    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        LOGE("ERROR: downloadFile failed with error: %s (%d)", av_err2str(ret), ret);
        LOGE("========== downloadFile FAILED ==========");
        return 1;
    }

    LOGE("========== downloadFile SUCCESS ==========");
    LOGE("Total packets processed: %d", packet_count);
    LOGE("Total bytes written: %lld", total_bytes_written);
    return 0;
}


/**
 * 手动下载视频保存为mp4.
 * flv -> mp4.
 * @param input
 * @param output
 * @return
 */
int downloadFileAAc(const char* input, const char* output){
    LOGE("========== downloadFileAAc START ==========");
    LOGE("Input: %s", input);
    LOGE("Output: %s", output);
//    AVAudioFifo
    int ret;
    //初始化一个空的packet并手动赋值给data和size这两个变量.
    AVPacket packet = { .data = NULL, .size = 0 };
    AVFrame *frame = NULL;
    //流类型.
    enum AVMediaType type;
    unsigned int stream_index;
    unsigned int i;
    int got_frame;
    int packet_count = 0;
    int64_t total_bytes_written = 0;
    //预定义一个空方法. 函数指针.
    int (*dec_func)(AVCodecContext *, AVFrame *, int *, const AVPacket *);
    //音频转换上下文.
    SwrContext *resample_context = NULL;
    //音频转换缓冲池.
    AVAudioFifo *fifo = NULL;
    //开始解码tag 1：开始转码 0：结束finish.
    isDownloading = 1;

    //打开输入流.
    LOGE("Opening input file: %s", input);
    if ((ret = open_input_file(input)) < 0) {
        LOGE("ERROR: Failed to open input file '%s', error: %s (%d)", input, av_err2str(ret), ret);
        goto end;
    }
    LOGE("Input file opened successfully, streams: %d", ifmt_ctx ? ifmt_ctx->nb_streams : 0);
    
    //打开输出流.
    LOGE("Opening output file: %s", output);
    if ((ret = open_output_file(output)) < 0) {
        LOGE("ERROR: Failed to open output file '%s', error: %s (%d)", output, av_err2str(ret), ret);
        goto end;
    }
    LOGE("Output file opened successfully");
    
    //初始化过滤器 
    /* Initialize the resampler to be able to convert audio sample formats. */
    // 找到音频流的索引
    LOGE("Searching for audio stream...");
    AVCodecContext *input_audio_ctx = NULL;
    AVCodecContext *output_audio_ctx = NULL;
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        LOGE("Checking stream %u: dec_ctx=%p, codec_type=%d", i, 
             stream_ctx ? stream_ctx[i].dec_ctx : NULL,
             stream_ctx && stream_ctx[i].dec_ctx ? stream_ctx[i].dec_ctx->codec_type : -1);
        if (stream_ctx && stream_ctx[i].dec_ctx && 
            stream_ctx[i].dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            input_audio_ctx = stream_ctx[i].dec_ctx;
            LOGE("Found audio decoder context at index %u", i);
            if (stream_ctx[i].enc_ctx) {
                output_audio_ctx = stream_ctx[i].enc_ctx;
                LOGE("Found audio encoder context at index %u", i);
            } else {
                LOGE("WARNING: Audio encoder context is NULL at index %u", i);
            }
            break;
        }
    }
    
    if (!input_audio_ctx) {
        LOGE("ERROR: Could not find audio decoder context");
        ret = AVERROR_STREAM_NOT_FOUND;
        goto end;
    }
    
    if (!output_audio_ctx) {
        LOGE("ERROR: Could not find audio encoder context");
        ret = AVERROR_STREAM_NOT_FOUND;
        goto end;
    }
    
    LOGE("Initializing resampler...");
    LOGE("Input audio: sample_rate=%d, sample_fmt=%d, channels=%d", 
         input_audio_ctx->sample_rate, input_audio_ctx->sample_fmt, 
         input_audio_ctx->ch_layout.nb_channels);
    LOGE("Output audio: sample_rate=%d, sample_fmt=%d, channels=%d", 
         output_audio_ctx->sample_rate, output_audio_ctx->sample_fmt, 
         output_audio_ctx->ch_layout.nb_channels);
    
    if (init_resampler(input_audio_ctx, output_audio_ctx, &resample_context)) {
        LOGE("ERROR: Failed to initialize resampler");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    LOGE("Resampler initialized successfully");
    
    /* Initialize the FIFO buffer to store audio samples to be encoded. */
    LOGE("Initializing FIFO buffer...");
    if (init_fifo(&fifo, output_audio_ctx)) {
        LOGE("ERROR: Failed to initialize FIFO buffer");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    LOGE("FIFO buffer initialized successfully");

    /* Note: open_output_file already writes the header, so we skip this step */
    LOGE("Output file header already written in open_output_file");


    /* read all packets */
    LOGE("Starting packet reading loop...");
    while (isDownloading) {

        /* Use the encoder's desired frame size for processing. */
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0) {
            if (ret == AVERROR_EOF) {
                LOGE("End of input stream reached");
            } else {
                LOGE("ERROR: Failed to read frame, error: %s (%d)", av_err2str(ret), ret);
            }
            break;
        }
        stream_index = packet.stream_index;
        packet_count++;
        
        if (packet_count % 100 == 0) {
            LOGE("Processed %d packets, total bytes: %lld", packet_count, total_bytes_written);
        }

        //获取解码器上下文.
        if (stream_index < ifmt_ctx->nb_streams && stream_ctx && stream_ctx[stream_index].dec_ctx) {
            AVCodecContext* dec_ctx = stream_ctx[stream_index].dec_ctx;
            //计算一个framesize.
            const int output_frame_size = dec_ctx->frame_size;
        }

        type = ifmt_ctx->streams[packet.stream_index]->codecpar->codec_type;
        const char *codec_type_name = av_get_media_type_string(type);
        LOGI("Demuxer gave frame of stream_index %u, type=%s", stream_index, codec_type_name ? codec_type_name : "unknown");
        
        //处理过滤器.
        /* remux this frame without reencoding */
        av_packet_rescale_ts(&packet,
                             ifmt_ctx->streams[stream_index]->time_base,
                             ofmt_ctx->streams[stream_index]->time_base);
        ret = av_interleaved_write_frame(ofmt_ctx, &packet);
        if (ret < 0) {
            LOGE("ERROR: Failed to mux packet #%d, error: %s (%d)", packet_count, av_err2str(ret), ret);
            goto end;
        }
        total_bytes_written += packet.size;
        av_packet_unref(&packet);
    }
    
    LOGE("Packet reading loop ended. Total packets: %d, Total bytes: %lld", packet_count, total_bytes_written);

    /* flush filters and encoders */
    LOGE("Flushing filters and encoders...");
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        /* flush filter */
        if (!filter_ctx[i].filter_graph) {
            LOGI("Stream %u: no filter graph, skipping filter flush", i);
            continue;
        }
        LOGI("Flushing filter for stream %u", i);
        ret = filter_encode_write_frame(NULL, i);
        if (ret < 0) {
            LOGE("ERROR: Flushing filter failed for stream %u, error: %s (%d)", i, av_err2str(ret), ret);
            goto end;
        }

        /* flush encoder */
        LOGI("Flushing encoder for stream %u", i);
        ret = flush_encoder(i);
        if (ret < 0) {
            LOGE("ERROR: Flushing encoder failed for stream %u, error: %s (%d)", i, av_err2str(ret), ret);
            goto end;
        }
    }
    LOGE("Filters and encoders flushed successfully");

    LOGE("Writing output file trailer...");
    ret = av_write_trailer(ofmt_ctx);
    if (ret < 0) {
        LOGE("ERROR: Failed to write trailer, error: %s (%d)", av_err2str(ret), ret);
    } else {
        LOGE("Output file trailer written successfully");
    }
    
    end:
    LOGE("========== downloadFileAAc CLEANUP ==========");
    
    LOGE("Cleaning up resources...");
    av_packet_unref(&packet);
    av_frame_free(&frame);
    
    if (resample_context) {
        LOGE("Freeing resampler context...");
        swr_free(&resample_context);
    }
    
    if (fifo) {
        LOGE("Freeing FIFO buffer...");
        av_audio_fifo_free(fifo);
    }
    
    if (ifmt_ctx) {
        LOGE("Closing input context...");
        for (i = 0; i < ifmt_ctx->nb_streams; i++) {
            if (stream_ctx && stream_ctx[i].dec_ctx) {
                avcodec_free_context(&stream_ctx[i].dec_ctx);
            }
            if (ofmt_ctx && ofmt_ctx->nb_streams > i && ofmt_ctx->streams[i] && stream_ctx && stream_ctx[i].enc_ctx)
                avcodec_free_context(&stream_ctx[i].enc_ctx);
            if (filter_ctx && filter_ctx[i].filter_graph)
                avfilter_graph_free(&filter_ctx[i].filter_graph);
        }
        avformat_close_input(&ifmt_ctx);
    }
    
    if (filter_ctx) {
        LOGE("Freeing filter context...");
        av_free(filter_ctx);
    }
    
    if (stream_ctx) {
        LOGE("Freeing stream context...");
        av_free(stream_ctx);
    }
    
    if (ofmt_ctx) {
        if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
            LOGE("Closing output file I/O...");
            avio_closep(&ofmt_ctx->pb);
        }
        LOGE("Freeing output context...");
        avformat_free_context(ofmt_ctx);
    }

    if (ret < 0){
        LOGE("ERROR: downloadFileAAc failed with error: %s (%d)", av_err2str(ret), ret);
        LOGE("========== downloadFileAAc FAILED ==========");
        return 1;
    }

    LOGE("========== downloadFileAAc SUCCESS ==========");
    LOGE("Total packets processed: %d", packet_count);
    LOGE("Total bytes written: %lld", total_bytes_written);
    return 0;

}

//打开输入文件.获取解码器.
int open_input_file(const char *filename) {
    int ret;
    unsigned int i;
    //init the network .
    avformat_network_init();

    //总上下文，用来解压视频为 视频流+音频流.
    ifmt_ctx = avformat_alloc_context();
    //设置读取参数
    AVDictionary* inputDic = NULL ;
    av_dict_set(&inputDic, "rtsp_transport", "tcp", 0);

    if ((ret = avformat_open_input(&ifmt_ctx, filename, 0, &inputDic)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        LOGE("Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        LOGE("Cannot find stream information\n");
        return ret;
    }

    //init stream_ctx .
//    stream_ctx = av_mallocz_array(ifmt_ctx->nb_streams, sizeof(*stream_ctx));
    stream_ctx = av_malloc_array(ifmt_ctx->nb_streams, sizeof(*stream_ctx));
    memset(stream_ctx , 0 , ifmt_ctx->nb_streams * sizeof(*stream_ctx));

    if (!stream_ctx){
        LOGE("init stream_ctx error!");
        return AVERROR(ENOMEM);
    }

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *stream = ifmt_ctx->streams[i];

//        AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
        //3.1 找解码参数
        AVCodecParameters* codecParameters = stream->codecpar;
        //3.2 找解码器
        AVCodec* dec = avcodec_find_decoder(codecParameters->codec_id);
        if (!dec) {
            av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
            LOGE("Failed to find decoder for stream #%u\n", i);
            return AVERROR_DECODER_NOT_FOUND;
        }
        //解码器上下文.
        AVCodecContext *codec_ctx = avcodec_alloc_context3(dec);
        if (!codec_ctx) {
            av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
            LOGE("Failed to allocate the decoder context for stream #%u\n", i);
            return AVERROR(ENOMEM);
        }

        //给解码器增加参数.
        ret = avcodec_parameters_to_context(codec_ctx, codecParameters);

        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
                                       "for stream #%u\n", i);
            LOGE("Failed to copy decoder parameters to input decoder context "
                 "for stream #%u\n", i);
            return ret;
        }
        /* Reencode video & audio and remux subtitles etc. */
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
            || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
                codec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, stream, NULL);
            /* Open decoder */
            ret = avcodec_open2(codec_ctx, dec, NULL);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
                LOGE("Failed to open decoder for stream #%u\n", i);
                return ret;
            }
        }
        stream_ctx[i].dec_ctx = codec_ctx;
    }

    av_dump_format(ifmt_ctx, 0, filename, 0);
    return 0;
}

/**
 * 输出文件打开，初始化编码器上下文.
 * @param filename 输出文件路径.
 * @return
 */
int open_output_file(const char *filename) {
    AVStream *out_stream;
    AVStream *in_stream;
    AVCodecContext *dec_ctx, *enc_ctx;
    AVCodec *encoder;
    int ret;
    unsigned int i;

    LOGE("open_output_file:%s ", filename);

    //AvFormatCotext. 输出上下文.
    ofmt_ctx = avformat_alloc_context();
    //int the out put avformat context .
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);

    if (!ofmt_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
        LOGE("Could not create output context\n");
        return AVERROR_UNKNOWN;
    }


    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        in_stream = ifmt_ctx->streams[i];

        //construct the output stream .
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
            LOGE("Failed allocating output stream\n");
            return AVERROR_UNKNOWN;
        }
        //decode context.
        dec_ctx = stream_ctx[i].dec_ctx;
        if(!dec_ctx){
            LOGE("解码器%s为NULL",i);
        }

        //attention: get the codec failed lead to failed convert flv stream file.
        if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
            || dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {

            if(dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO){//视频采用h264编码over.
                LOGE("AVMEDIA_TYPE_VIDEO");
                if(dec_ctx->codec_id == AV_CODEC_ID_HEVC){
                    LOGE("the decode video type is HEVC~!");
                }

                encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
                if (!encoder) {
                    LOGE("H264 encoder get failed ! ");
                }else{
                    LOGE("H264 encoder get success ! ");
                }

            }else if(dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO){//音频采用AAC
                LOGE("AVMEDIA_TYPE_AUDIO");
                if(dec_ctx->codec_id == AV_CODEC_ID_PCM_ALAW){
                    LOGE("the decode audio type is PCMA~! ");
//                    todo:要重采样PCMA为AAC格式。
                    encoder = avcodec_find_encoder(AV_CODEC_ID_AAC);
                    if(!encoder){
                        LOGE("AAC encoder get failed !");
                    }else{
                        LOGE("AAC encoder get success !");
                    }
                }else{
                    encoder = avcodec_find_encoder(dec_ctx->codec_id);
                }
                if (!encoder) {
                    LOGE("audio encoder get failed !");
                }
            }else{
                LOGE("UNKNOWN_TYPE_AUDIO");
                encoder = avcodec_find_encoder(dec_ctx->codec_id);
            }
            /* in this example, we choose transcoding to same codec */
//            encoder = avcodec_find_encoder(dec_ctx->codec_id);

            if (!encoder) {
                av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
                LOGE("Necessary encoder not found\n");
                return AVERROR_INVALIDDATA;
            }

            //编码上下文.
            enc_ctx = avcodec_alloc_context3(encoder);

            if (!enc_ctx) {
                av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
                LOGE("Failed to allocate the encoder context\n");
                return AVERROR(ENOMEM);
            }

//            enc_ctx->codec_id = encoder->id;
//            enc_ctx->codec_type = dec_ctx->codec_type;
            /* In this example, we transcode to same properties (picture size,
             * sample rate etc.). These properties can be changed for output
             * streams easily using filters */
            if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                enc_ctx->height = dec_ctx->height;
                enc_ctx->width = dec_ctx->width;
                enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
                /* take first format from list of supported formats */
                if (encoder->pix_fmts)
                    enc_ctx->pix_fmt = encoder->pix_fmts[0];
                else
                    enc_ctx->pix_fmt = dec_ctx->pix_fmt;
                /* video time_base can be set to whatever is handy and supported by encoder */
                enc_ctx->time_base = av_inv_q(dec_ctx->framerate);
            } else {
                enc_ctx->sample_rate = 44100;//dec_ctx->sample_rate;
                // Use AVChannelLayout instead of deprecated channel_layout and channels
                av_channel_layout_default(&enc_ctx->ch_layout, 2);
                /* take first format from list of supported formats */
                enc_ctx->sample_fmt = encoder->sample_fmts[0];
                enc_ctx->time_base.den = dec_ctx->sample_rate;
                enc_ctx->time_base.num = 1;
//                enc_ctx->time_base = (AVRational){1, enc_ctx->sample_rate};
            }

            /* 打开输出编解码器，灯下面用 */
            if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                LOGE("Opening video encoder for stream #%u (codec: %s, width: %d, height: %d, pix_fmt: %d)", 
                     i, encoder->name, enc_ctx->width, enc_ctx->height, enc_ctx->pix_fmt);
            } else {
                LOGE("Opening audio encoder for stream #%u (codec: %s, sample_rate: %d, channels: %d, sample_fmt: %d)", 
                     i, encoder->name, enc_ctx->sample_rate, enc_ctx->ch_layout.nb_channels, enc_ctx->sample_fmt);
            }
            ret = avcodec_open2(enc_ctx, encoder, NULL);

            if (ret < 0) {
                LOGE("ERROR: Cannot open encoder for stream #%u, error: %s (%d)", i, av_err2str(ret), ret);
                return ret;
            }
            LOGE("Encoder opened successfully for stream #%u", i);
            //配置输出参数信息.
            ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", i);
                LOGE("Failed to copy encoder parameters to output stream #%u\n", i);
                return ret;
            }
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            //设置输出流的time base.
            if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                // 视频流使用编码器的time_base
                out_stream->time_base = enc_ctx->time_base;
                LOGE("Stream %u (video): time_base set to %d/%d", i, out_stream->time_base.num, out_stream->time_base.den);
            } else {
                // 音频流使用sample_rate
                out_stream->time_base.den = dec_ctx->sample_rate;
                out_stream->time_base.num = 1;
                LOGE("Stream %u (audio): time_base set to %d/%d", i, out_stream->time_base.num, out_stream->time_base.den);
            }

            stream_ctx[i].enc_ctx = enc_ctx;
        } else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            av_log(NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type, cannot proceed\n", i);
            LOGE("Elementary stream #%d is of unknown type, cannot proceed\n", i);
            return AVERROR_INVALIDDATA;
        } else {
            /* if this stream must be remuxed */
            ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Copying parameters for stream #%u failed\n", i);
                LOGE("Copying parameters for stream #%u failed\n", i);
                return ret;
            }
            out_stream->time_base = in_stream->time_base;
        }

    }
    LOGE("Total output streams created: %d", ifmt_ctx->nb_streams);
    av_dump_format(ofmt_ctx, 0, filename, 1);

    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        LOGE("Opening output file I/O: %s", filename);
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGE("ERROR: Could not open output file '%s', error: %s (%d)", filename, av_err2str(ret), ret);
            return ret;
        }
        LOGE("Output file I/O opened successfully");
    } else {
        LOGE("Output format does not require file I/O");
    }

    /* init muxer, write output file header */
    LOGE("Writing output file header in open_output_file...");
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        LOGE("ERROR: Failed to write output file header, error: %s (%d)", av_err2str(ret), ret);
        return ret;
    }
    LOGE("Output file header written successfully in open_output_file");

    return 0;
}


/**
 * 写入数据.
 * @param filt_frame
 * @param stream_index
 * @param got_frame
 * @return
 */
int encode_write_frame(AVFrame *filt_frame, unsigned int stream_index, int *got_frame) {
    int ret;
    int got_frame_local;
    AVPacket *enc_pkt;
    AVCodecContext *enc_ctx = stream_ctx[stream_index].enc_ctx;

    if (!got_frame)
        got_frame = &got_frame_local;

    LOGI("Encoding frame\n");

    /* send frame to encoder */
    ret = avcodec_send_frame(enc_ctx, filt_frame);
    if (filt_frame) {
        av_frame_free(&filt_frame);
    }
    if (ret < 0) {
        if (ret == AVERROR_EOF) {
            /* encoder already flushed, nothing to send */
            *got_frame = 0;
            return 0;
        } else if (ret != AVERROR(EAGAIN)) {
            LOGE("Error sending frame for encoding: %s\n", av_err2str(ret));
            return ret;
        }
        /* EAGAIN means encoder needs output to be read first */
    }

    /* receive encoded packets */
    enc_pkt = av_packet_alloc();
    if (!enc_pkt) {
        LOGE("Could not allocate packet\n");
        return AVERROR(ENOMEM);
    }

    *got_frame = 0;
    ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, enc_pkt);
        if (ret == AVERROR(EAGAIN)) {
            /* need more input */
            av_packet_free(&enc_pkt);
            return 0;
        } else if (ret == AVERROR_EOF) {
            /* encoder fully flushed, no more output */
            av_packet_free(&enc_pkt);
            return 0;
        } else if (ret < 0) {
            LOGE("Error during encoding: %s\n", av_err2str(ret));
            av_packet_free(&enc_pkt);
            return ret;
        }

        *got_frame = 1;

        /* prepare packet for muxing */
        enc_pkt->stream_index = stream_index;
        av_packet_rescale_ts(enc_pkt,
                             enc_ctx->time_base,
                             ofmt_ctx->streams[stream_index]->time_base);

        LOGI("Muxing frame\n");
        /* mux encoded frame */
        ret = av_interleaved_write_frame(ofmt_ctx, enc_pkt);
        av_packet_unref(enc_pkt);
        if (ret < 0) {
            av_packet_free(&enc_pkt);
            return ret;
        }
    }

    av_packet_free(&enc_pkt);
    return 0;
}

int filter_encode_write_frame(AVFrame *frame, unsigned int stream_index) {
    int ret;
    AVFrame *filt_frame;

    LOGI("Pushing decoded frame to filters\n");
    /* push the decoded frame into the filtergraph */
    ret = av_buffersrc_add_frame_flags(filter_ctx[stream_index].buffersrc_ctx,
                                       frame, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
        return ret;
    }

    /* pull filtered frames from the filtergraph */
    while (1) {
        filt_frame = av_frame_alloc();
        if (!filt_frame) {
            ret = AVERROR(ENOMEM);
            break;
        }
        LOGI("Pulling filtered frame from filters\n");
        ret = av_buffersink_get_frame(filter_ctx[stream_index].buffersink_ctx,
                                      filt_frame);
        if (ret < 0) {
            /* if no more frames for output - returns AVERROR(EAGAIN)
             * if flushed and no more frames for output - returns AVERROR_EOF
             * rewrite retcode to 0 to show it as normal procedure completion
             */
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                ret = 0;
            av_frame_free(&filt_frame);
            break;
        }

        filt_frame->pict_type = AV_PICTURE_TYPE_NONE;
        ret = encode_write_frame(filt_frame, stream_index, NULL);
        if (ret < 0)
            break;
    }

    return ret;
}

int flush_encoder(unsigned int stream_index) {
    int ret;
    int got_frame;

    if (!(stream_ctx[stream_index].enc_ctx->codec->capabilities &
          AV_CODEC_CAP_DELAY))
        return 0;

    while (1) {
        LOGI("Flushing stream #%u encoder\n", stream_index);
        ret = encode_write_frame(NULL, stream_index, &got_frame);
        if (ret < 0)
            break;
        if (!got_frame)
            return 0;
    }
    return ret;
}

JNIEXPORT jint JNICALL
Java_com_jdpxiaoming_ffmpeg_1cmd_FFmpegCmd_dump_1Rtsp_1h265(JNIEnv *env, jclass clazz,
                                                            jstring input, jstring output) {
    LOGE("Java_com_jdpxiaoming_ffmpeg_1cmd_FFmpegCmd_dump_1Rtsp_1h265()~");
    inputPath = (char *) (*env)->GetStringUTFChars(env, input, 0);
    outputPath = (char *) (*env)->GetStringUTFChars(env, output, 0);
    pthread_create(&pid_dump, NULL ,startRtspDownloadThread, NULL);
}


/**
 * Write the header of the output file container.
 * @param output_format_context Format context of the output file
 * @return Error code (0 if successful)
 */
int write_output_file_header(AVFormatContext *output_format_context)
{
    int error;
    LOGE("write_output_file_header called");
    if ((error = avformat_write_header(output_format_context, NULL)) < 0) {
        LOGE("ERROR: Could not write output file header, error: %s (%d)", av_err2str(error), error);
        return error;
    }
    LOGE("write_output_file_header succeeded");
    return 0;
}


/**
 * 初始化Audio Fifo queue .
 * @param fifo
 * @param output_codec_context
 * @return
 */
int init_fifo(AVAudioFifo **fifo, AVCodecContext *output_codec_context){
    /* Create the FIFO buffer based on the specified output sample format. */
    // Use ch_layout.nb_channels instead of deprecated channels
    int nb_channels = output_codec_context->ch_layout.nb_channels;
    if (!(*fifo = av_audio_fifo_alloc(output_codec_context->sample_fmt,
                                      nb_channels, 1))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;

}

/**
 * 初始化SwrContext重采样上下文
 * @param input_codec_context
 * @param output_codec_context
 * @param resample_context
 * @return
 */
int init_resampler(AVCodecContext *input_codec_context,AVCodecContext *output_codec_context,
                   SwrContext **resample_context){
    int error;
    AVChannelLayout out_ch_layout = {0}, in_ch_layout = {0};

    LOGE("init_resampler: Starting resampler initialization");
    /*
     * Create a resampler context for the conversion.
     * Set the conversion parameters.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity (they are sometimes not detected
     * properly by the demuxer and/or decoder).
     */
    // Copy channel layout from codec context
    error = av_channel_layout_copy(&out_ch_layout, &output_codec_context->ch_layout);
    if (error < 0) {
        LOGE("ERROR: Could not copy output channel layout, error: %s (%d)", av_err2str(error), error);
        return error;
    }

    error = av_channel_layout_copy(&in_ch_layout, &input_codec_context->ch_layout);
    if (error < 0) {
        LOGE("ERROR: Could not copy input channel layout, error: %s (%d)", av_err2str(error), error);
        av_channel_layout_uninit(&out_ch_layout);
        return error;
    }

    // Use swr_alloc_set_opts2 instead of deprecated swr_alloc_set_opts
    error = swr_alloc_set_opts2(resample_context,
                                &out_ch_layout,
                                output_codec_context->sample_fmt,
                                output_codec_context->sample_rate,
                                &in_ch_layout,
                                input_codec_context->sample_fmt,
                                input_codec_context->sample_rate,
                                0, NULL);

    // Clean up channel layouts
    av_channel_layout_uninit(&out_ch_layout);
    av_channel_layout_uninit(&in_ch_layout);

    if (error < 0) {
        LOGE("ERROR: Could not allocate resample context, error: %s (%d)", av_err2str(error), error);
        return error;
    }
    /*
    * Perform a sanity check so that the number of converted samples is
    * not greater than the number of samples to be converted.
    * If the sample rates differ, this case has to be handled differently
    */
    if (output_codec_context->sample_rate != input_codec_context->sample_rate) {
        LOGE("WARNING: Sample rates differ: input=%d, output=%d. Resampling will occur.", 
             input_codec_context->sample_rate, output_codec_context->sample_rate);
    }

    /* Open the resampler with the specified parameters. */
    LOGE("Initializing swr context...");
    if ((error = swr_init(*resample_context)) < 0) {
        LOGE("ERROR: Could not open resample context, error: %s (%d)", av_err2str(error), error);
        swr_free(resample_context);
        return error;
    }
    LOGE("Resampler context initialized successfully");
    return 0;
}