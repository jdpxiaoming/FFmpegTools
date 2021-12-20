# FFmpegTools the FFmpeg Android Dynamic Library
> 想学习FFmpeg不知道如何开始，那么这个项目是为你准备的，使用过binary执行ffmpeg吗，那么本项目就是为你准备的进阶教程.


> use this library you can do all the things with ffmepg ex: video crop ,video filter ,video stream dump and so on .

####  use ffmpeg as the binary file use commands params to run main funcition , picture and videos acitons as you licke.

- [x] Transcode flv->mp4

- [x] Dump RTSP live Stream to Android SDcard/xx.mp4 file .

- [x] gif/image merge and so on .

- 配置maven
```
allprojects {
    
    repositories {
        maven { url 'https://repo1.maven.org/maven2/'}
        google()
        jcenter()
    }
 }
```

- 引用 32位

```groovy
implementation 'io.github.jdpxiaoming:ffmpeg-cmd:0.0.14'
```
- 64位

```groovy
implementation 'io.github.jdpxiaoming:ffmpeg-cmd64:0.0.14'
```

- 编译64位需要修改`ffmpeg/config.h`
  
```bash
#define ARCH_AARCH64 1
#define ARCH_ARM 0
```
- example:

```
        String inputPath = Environment.getExternalStorageDirectory().getAbsolutePath()+"/Download/58.flv";
        String outputPath = Environment.getExternalStorageDirectory().getAbsolutePath()+"/Download/61.mp4";
        File input =new File(inputPath);
        if(!input.exists()){
            Toast.makeText(MainActivity.this, "/Download/58.flv not found!", Toast.LENGTH_LONG).show();
            return;
        }
        File output =new File(outputPath);
        if(output.exists()){
            output.delete();
        }
        //cmds for ffmpeg flv->mp4.
        inputPath ="rtsp://47.108.81.159:5555/rtsp/992949a2-4d57-439f-8afb-9d940a13d786";
        String[] commands = FFmpegFactory.buildRtsp2Mp4(inputPath,outputPath);

        FFmpegUtil.getInstance().enQueueTask(commands, 0,new FFmpegUtil.onCallBack() {
            @Override
            public void onStart() {

                Log.i(TAG," onStart2 # ");
            }

            @Override
            public void onFailure() {
                Log.i(TAG," onFailure2 # ");
            }

            @Override
            public void onComplete() {
                Log.i(TAG," onComplete2 # ");
                Toast.makeText(MainActivity.this, "transcode successful2!", Toast.LENGTH_LONG).show();
            }

            @Override
            public void onProgress(float progress) {
                Log.i(TAG," onProgress2 # "+progress);
            }
        });

```

### 如果你想更高级一点，那么...
```
public void dumpFlv(View view) {
        String input = "http://118.31.174.18:5581/rtmp/8e5196c4-e7d9-41b0-9080-fa0da638d9e2/live.flv";
        String output = new File(Environment.getExternalStorageDirectory(),"/poe/output61.mp4").getAbsolutePath();
        FFmpegCmd.dump_stream(input , output);
    }
```
- 和上面的区别：`使用jni代码调用方式dump各种直播流为本地Mp4. `

- remuxing.c(来源于编译库后官方demo  目录/share,你也可以在本项目/docs/examples查看)

```c

/*
 * Copyright (c) 2013 Stefano Sabatini
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file
 * libavformat/libavcodec demuxing and muxing API example.
 *
 * Remux streams from one container format to another.
 * @example remuxing.c
 */

#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}

int main(int argc, char **argv)
{
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    const char *in_filename, *out_filename;
    int ret, i;
    int stream_index = 0;
    int *stream_mapping = NULL;
    int stream_mapping_size = 0;

    if (argc < 3) {
        printf("usage: %s input output\n"
               "API example program to remux a media file with libavformat and libavcodec.\n"
               "The output format is guessed according to the file extension.\n"
               "\n", argv[0]);
        return 1;
    }

    // get the input path .
    in_filename  = argv[1];
    // get the output path . 
    out_filename = argv[2];


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
    stream_mapping = av_mallocz_array(stream_mapping_size, sizeof(*stream_mapping));
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
    while (1) {
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
        log_packet(ifmt_ctx, &pkt, "in");

        /* copy packet */
        //caculate the time base of dpts & dts ,duration and pos . if mp4 has pos .
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        log_packet(ofmt_ctx, &pkt, "out");

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

```

### 学习资料
- [remuxing.c](/docs/examples/remuxing.c)
- [encode_audio.c](/docs/examples/encode_audio.c)
- [encode_video.c](/docs/examples/encode_video.c)
- [more examples ](/docs/examples/)

### How to build your own ffmpeg.so ?
- https://blog.lxfpoe.work/jekyll/update/2020/05/18/ffmpeg-so.html
- https://blog.lxfpoe.work/jekyll/update/2020/05/18/ffmpeg-android.html


