/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
#include <termios.h>
#include <include/libavutil/audio_fifo.h>
#include <include/libswresample/swresample.h>
#include "include/libavformat/avformat.h"
#include "include/libavfilter/avfilter.h"
/* Header for class JniTest_FFmpegCmd */

#ifndef _Included_FFmpeg_Cmd
#define _Included_FFmpeg_Cmd
//#ifdef __cplusplus
//extern "C" {
//#endif
//#ifdef __cplusplus
//}
//#endif
#endif

void ffmpeg_progress(float progress);
void ffmpeg_complete(int errorCode);
void ffmpeg_failure(int errorCode);
/**
 * dump stream to local mp4 file witch -vcodec copy -acodec copy .
 * @param input
 * @param output
 * @return
 */
int downloadFile(const char* input, const char* output);

/**
 * dum stream to mp4 file with -vcodec copy -acodec aac ouput.mp4 .
 * @param input
 * @param output
 * @return
 */
int downloadFileAAc(const char* input, const char* output);

//全局静态变量 .
static pthread_t pid_dump; //the thread id .use to interrupt the download thread .
static const char * inputPath; //input stream address .
static const char * outputPath; // the output file absolute path .
/**
 * 1： dump file is doing .
 * 0: stop dump and ready to exit .
 */
static int isDownloading ;

//全局静态变量.完整的音视频下载重采样为mp4.
/*
* input AvFormatContext.
*/
static AVFormatContext *ifmt_ctx;
/*
*output AvFormatContext
*/
static AVFormatContext *ofmt_ctx;
/*
* 过滤器包裹类 ,src过滤器上下文/sin过滤器上下文/AvFilterGraph
*/
typedef struct FilteringContext {
    AVFilterContext *buffersink_ctx;
    AVFilterContext *buffersrc_ctx;
    AVFilterGraph *filter_graph;
} FilteringContext;
/*
* 过滤器包裹类 ,src过滤器上下文/sin过滤器上下文/AvFilterGraph
*/
static FilteringContext *filter_ctx;

typedef struct StreamContext {
    AVCodecContext *dec_ctx;
    AVCodecContext *enc_ctx;
} StreamContext;
/*
* AvCodecContext 解码器/编码器上下文集合.
*/
static StreamContext *stream_ctx;

//定义一些公用functions.
//打开输入文件.获取解码器.
int open_input_file(const char *filename);
//打开输出文件并配置编码器.
int open_output_file(const char *filename);

/**
 * 写入文件头(ex:Mp4 File Header)
 * @param output_format_context
 * @return
 */
int write_output_file_header(AVFormatContext *output_format_context);

int init_fifo(AVAudioFifo **fifo, AVCodecContext *output_codec_context);
int init_resampler(AVCodecContext *input_codec_context,
AVCodecContext *output_codec_context,
        SwrContext **resample_context);


int encode_write_frame(AVFrame *filt_frame, unsigned int stream_index, int *got_frame);
int filter_encode_write_frame(AVFrame *frame, unsigned int stream_index);
int flush_encoder(unsigned int stream_index);