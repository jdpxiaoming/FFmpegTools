package com.jdpxiaoming.ffmpeg_cmd;

import android.util.Log;

import java.util.ArrayList;

/**
 * some ffmpeg cmds factory.
 */
public class FFmpegFactory {

    //device info .
    public static int screenWidth = 1280;
    public static int screenHeight = 720;
    public static int cameraWidth = 640;
    public static int cameraHeight = 360;
    public static int recodeWidth = 640;
    public static int recodeHeight = 360;
    //video setting .
    public static int titlePicWidth = 200;
    public static int titlePicHeight = 100;
    public static int titleDuration = 3;

    /**
     *  flv to mp4 .
     * @param inputPath
     * @param outputPath
     * @return
     */
    public static String[] buildFlv2Mp4(String inputPath ,String outputPath){
        String[] commands = new String[8];
        commands[0] = "ffmpeg";
        commands[1] = "-i";
        commands[2] = inputPath;
        commands[3] = "-vcodec";
        commands[4] = "copy";
        commands[5] = "-acodec";
        commands[6] = "aac";
        commands[7] = outputPath;
        return commands;
    }



    /**
     *  flv->mp4  .
     * @param inputPath
     * @param outputPath  .
     * @return
     */
    public static String[] buildRtsp2Mp4(String inputPath ,String outputPath){
        String[] commands = new String[15];
        commands[0] = "ffmpeg";
        commands[1] = "-rtsp_transport";
        commands[2] = "tcp";
        commands[3] = "-i";
        commands[4] = inputPath;
        commands[5] = "-vcodec";
        commands[6] = "copy";
        //       -tag:v hvc1
        commands[7] = "-tag:v";
        commands[8] = "hvc1";

        commands[9] = "-acodec";
        commands[10] = "aac";
        commands[11] = "-f";
        commands[12] = "mp4";
        commands[13] = "-y";//覆盖
        commands[14] = outputPath;

        return commands;
    }


    /**
     * zoom scale video
     *
     * @param inputPath
     * @param outputPath
     * @param width
     * @param height
     * @return
     */
    public static String[] scaleVideo(String inputPath, String outputPath, int width, int height) {
        String[] commands = new String[6];
        commands[0] = "ffmpeg";
        commands[1] = "-i";
        commands[2] = inputPath;
        commands[3] = "-vf";
        commands[4] = "scale=" + width + ":" + height;
        commands[5] = outputPath;
        return commands;
    }


    //ffmpeg -ss 00:00:01 -t 3 -i input.mp4 -vf crop=iw:ih*2/3 -s 320x240 -r 7 output.gif
    //ffmpeg -t 3 -ss 00:00:02 -i small.mp4 small-clip.gif
    public static String[] cutVideoGif(String inputPath, String outputPath, String duration, String fps, String wh) {
        String[] commands = new String[12];
        commands[0] = "ffmpeg";
        commands[1] = "-ss";
        commands[2] = "00:00:00";
        commands[3] = "-t";
        commands[4] = duration;
        commands[5] = "-i";
        commands[6] = inputPath;
        commands[7] = "-s";
        commands[8] = wh;
        commands[9] = "-r";
        commands[10] = fps;
        commands[11] = outputPath;
        return commands;
    }

    public static String[] cutVideoGif(String inputPath, String outputPath, String duration) {
        String[] commands = new String[8];
        commands[0] = "ffmpeg";
        commands[1] = "-ss";
        commands[2] = "00:00:00";
        commands[3] = "-t";
        commands[4] = duration;
        commands[5] = "-i";
        commands[6] = inputPath;
        commands[7] = outputPath;
        return commands;
    }

    public static String[] cutVideoTime(String path, int startTime, int endTime, String output) {
        //ffmpeg -ss 00:00:15 -t 00:00:05 -i input.mp4 -vcodec copy -acodec copy output.mp4
        //ffmpeg -ss **START_TIME** -i input.mp4  -t **STOP_TIME** -acodec copy -vcodec copy output.mp4
        int startM = startTime / 1000;
        int endM = (endTime - startTime) / 1000;

        String startStr;
        String endStr;

        if (startM < 10) {
            startStr = "00:00:0" + startM;
        } else {
            startStr = "00:00:" + startM;
        }

        if (endM < 10) {
            endStr = "00:00:0" + endM;
        } else {
            endStr = "00:00:" + endM;
        }

        String[] commands = new String[12];
        commands[0] = "ffmpeg";
        commands[1] = "-i";
        commands[2] = path;
        commands[3] = "-vcodec";
        commands[4] = "copy";
        commands[5] = "-acodec";
        commands[6] = "copy";
        commands[7] = "-ss";
        commands[8] = startStr;
        commands[9] = "-t";
        commands[10] = endStr;
        commands[11] = output;
        return commands;
    }

    public static String[] addWaterMark(String imageUrl, String videoUrl, String outputUrl) {
//        # libx264 最快速度35秒
        String[] commands = new String[24];
        commands[0] = "ffmpeg";
        //input
        commands[1] = "-i";
        commands[2] = videoUrl;
        //water cover.
        commands[3] = "-i";
        commands[4] = imageUrl;
        commands[5] = "-filter_complex";
        commands[6] = "[1:v] fade=out:st=30:d=1:alpha=1 [ov]; [0:v][ov] overlay=(main_w-overlay_w)/2:(main_h-overlay_h)/2  [v]" ;
        commands[7] = "-map";
        commands[8] = "[v]";
        commands[9] = "-map";
        commands[10] = "0:a";
        commands[11] = "-c:v";
        commands[12] = "libx264";
        commands[13] = "-c:a";
        commands[14] = "copy";
        commands[15] = "-r";
        commands[16] = "15";
        commands[17] = "-crf";
        commands[18] = "28";
        commands[19] = "-shortest";
        commands[20] = "-preset";
        commands[21] = "ultrafast";
        //override -vcodec h264 -f mp4
        commands[22] = "-y";
        //output .
        commands[23] = outputUrl;


        /*String[] commands = new String[16];
        commands[0] = "ffmpeg";
        //input
        commands[1] = "-i";
        commands[2] = videoUrl;
        //water cover.
        commands[3] = "-i";
        commands[4] = imageUrl;
        commands[5] = "-filter_complex";
        commands[6] = "overlay=(main_w-overlay_w)/2:(main_h-overlay_h)/2";
        commands[7] = "-r";
        commands[8] = "15";
        //override -vcodec h264 -f mp4
        commands[9] = "-preset";
        commands[10] = "ultrafast";
        commands[11] = "-crf";
        commands[12] = "28";
        commands[13] = "-shortest";
        commands[14] = "-y";
        //output .
        commands[15] = outputUrl;*/

        return commands;
    }

    /**
     * picture water hint .
     */
    public static String[] addImageMark(String videoUrl, String imageUrl, String outputUrl) {
        String[] commands = new String[9];
        commands[0] = "ffmpeg";
        //input
        commands[1] = "-i";
        commands[2] = videoUrl;
        //water hint .
        commands[3] = "-i";
        commands[4] = imageUrl;
        commands[5] = "-filter_complex";
        commands[6] = "[1:v]scale=" + screenWidth + ":" + screenHeight + "[s];[0:v][s]overlay=0:0";
        commands[7] = "-y";
        commands[8] = outputUrl;
        return commands;
    }

    public static String[] addImageMark(String videoUrl, String imageUrl, String outputUrl, int width, int height) {
        String[] commands = new String[9];
        commands[0] = "ffmpeg";
        commands[1] = "-i";
        commands[2] = videoUrl;
        commands[3] = "-i";
        commands[4] = imageUrl;
        commands[5] = "-filter_complex";
        commands[6] = "[1:v]scale=" + width + ":" + height + "[s];[0:v][s]overlay=0:0";
        commands[7] = "-y";
        commands[8] = outputUrl;
        return commands;
    }

    /**
     * bgm .
     */
    public static String[] addMusic(String videoUrl, String musicUrl, String outputUrl) {
        String[] commands = new String[7];
        commands[0] = "ffmpeg";
        commands[1] = "-i";
        commands[2] = videoUrl;
        commands[3] = "-i";
        commands[4] = musicUrl;
        commands[5] = "-y";
        commands[6] = outputUrl;
        return commands;
    }

    /**
     * word hint .
     */
    public static String[] addTextMark(String videoUrl, String imageUrl, String outputUrl) {
        ArrayList<String> _commands = new ArrayList<>();
        _commands.add("ffmpeg");
        _commands.add("-i");
        _commands.add(videoUrl);
        _commands.add("-i");
        _commands.add(imageUrl);
        _commands.add("-filter_complex");
        _commands.add("overlay=(main_w-overlay_w)/2:(main_h-overlay_h)/2");
        _commands.add("-y");
        _commands.add(outputUrl);
        String[] commands = new String[_commands.size()];
        for (int i = 0; i < _commands.size(); i++) {
            commands[i] = _commands.get(i);
        }
        return commands;
    }

    /**
     * concat video with another video .
     */
    public static String[] concatVideo(String _filePath, String _outPath) {//-f concat -i list.txt -c copy concat.mp4
        ArrayList<String> _commands = new ArrayList<>();
        _commands.add("ffmpeg");
        _commands.add("-f");
        _commands.add("concat");
        _commands.add("-i");
        _commands.add(_filePath);
        _commands.add("-c");
        _commands.add("copy");
        _commands.add(_outPath);
        String[] commands = new String[_commands.size()];
        String _pr = "";
        for (int i = 0; i < _commands.size(); i++) {
            commands[i] = _commands.get(i);
            _pr += commands[i];
        }
        Log.d("LOGCAT", "ffmpeg command:" + _pr + "-" + commands.length);
        return commands;
    }

    /**
     * make video with many pictures.
     */
    public static String[] image2mov(String imageUrl, String _t, String outputUrl) {
        ArrayList<String> _commands = new ArrayList<>();
        _commands.add("ffmpeg");
        String _type = imageUrl.substring(imageUrl.length() - 3);
        if (_type.equals("gif")) {
            _commands.add("-ignore_loop");
            _commands.add("0");
        } else {
            _commands.add("-loop");
            _commands.add("1");
        }
        _commands.add("-i");
        _commands.add(imageUrl);
        //_commands.add("-vcodec");
        //_commands.add("libx264");
        _commands.add("-r");
        _commands.add("25");
        _commands.add("-b");
        _commands.add("200k");
        _commands.add("-s");
        _commands.add("640x360");
        _commands.add("-t");
        _commands.add(_t);
        _commands.add("-y");
        _commands.add(outputUrl);
        String[] commands = new String[_commands.size()];
        String _pr = "";
        for (int i = 0; i < _commands.size(); i++) {
            commands[i] = _commands.get(i);
            _pr += commands[i];
        }
        Log.d("LOGCAT", "ffmpeg command:" + _pr + "-" + commands.length);
        return commands;
    }

    /**
     * make video .
     */
    public static String[] makeVideo(String textImageUrl, String imageUrl, String musicUrl, String videoUrl, String outputUrl, int _duration) {
        ArrayList<String> _commands = new ArrayList<>();
        _commands.add("ffmpeg");
        _commands.add("-i");
        _commands.add(videoUrl);
        if (!textImageUrl.equals("") || !imageUrl.equals("")) {
            //picture hint .
            if (!imageUrl.equals("")) {
                _commands.add("-ignore_loop");
                _commands.add("0");
                _commands.add("-i");
                _commands.add(imageUrl);
            }
            if (!textImageUrl.equals("")) {
                _commands.add("-i");
                _commands.add(textImageUrl);
            }
            _commands.add("-filter_complex");
            if (textImageUrl.equals("")) {
                _commands.add("[1:v]scale=" + screenWidth + ":" + screenHeight + "[s];[0:v][s]overlay=0:0");
            } else if (imageUrl.equals("")) {
                _commands.add("overlay=x='if(lte(t," + titleDuration + "),(main_w-overlay_w)/2,NAN )':(main_h-overlay_h)/2");
            } else {
                _commands.add("[1:v]scale=" + screenWidth + ":" + screenHeight
                        + "[img1];[2:v]scale=" + titlePicWidth + ":" + titlePicHeight
                        + "[img2];[0:v][img1]overlay=0:0[bkg];[bkg][img2]overlay=x='if(lte(t,"
                        + titleDuration + "),(main_w-overlay_w)/2,NAN )':(main_h-overlay_h)/2");
            }
        }
        //music.
        if (!musicUrl.equals("")) {
            //-ss&-t control music length .
            //_commands.add("-ss");
            //_commands.add("00:00:00");
            //_commands.add("-t");
            //_commands.add(""+_duration);
            _commands.add("-i");
            _commands.add(musicUrl);
        }
        _commands.add("-r");
        _commands.add("25");
        _commands.add("-b");
        _commands.add("1000k");
        _commands.add("-s");
        _commands.add("640x360");
        //_commands.add("-y");
        _commands.add("-ss");
        _commands.add("00:00:00");
        _commands.add("-t");
        _commands.add("" + _duration);
        _commands.add(outputUrl);
        String[] commands = new String[_commands.size()];
        String _pr = "";
        for (int i = 0; i < _commands.size(); i++) {
            commands[i] = _commands.get(i);
            _pr += commands[i];
        }
        Log.d("LOGCAT", "ffmpeg command:" + _pr + commands.length);
        return commands;
    }
}
