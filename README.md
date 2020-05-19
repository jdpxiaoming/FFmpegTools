# FFmpegTools the FFmpeg Android Dynamic Library

> use this library you can do all the things with ffmepg ex: video crop ,video filter ,video stream dump and so on .

 use ffmpege as the binary file use commands params to run main funcition , picture and videos acitons as you licke.


- [x] Transcode flv->mp4

- [x] Dump RTSP live Stream to Android SDcard/xx.mp4 file .

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

        FFmpegCmd.exec(commands, new FFmpegCmd.OnCmdExecListener() {
            @Override
            public void onSuccess() {

            }

            @Override
            public void onFailure() {

            }

            @Override
            public void onComplete() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "transcode successful!", Toast.LENGTH_LONG).show();
                    }
                });
            }

            @Override
            public void onProgress(float progress) {

            }
        });

```



### How to build your own ffmpeg.so ?
- https://blog.lxfpoe.work/jekyll/update/2020/05/18/ffmpeg-so.html
- https://blog.lxfpoe.work/jekyll/update/2020/05/18/ffmpeg-android.html


