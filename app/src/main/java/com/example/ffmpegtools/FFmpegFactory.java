package com.example.ffmpegtools;

/**
 * 生产几个ffmpeg的命令集合.
 */
public class FFmpegFactory {

    /**
     *  flv->mp4命令 .
     * @param inputPath 流输入地址
     * @param outputPath 本地MP4保存路径.
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
     *  flv->mp4命令 .
     * @param inputPath 流输入地址
     * @param outputPath 本地MP4保存路径.
     * @return
     */
    public static String[] buildRtsp2Mp4(String inputPath ,String outputPath){
        String[] commands = new String[13];
        commands[0] = "ffmpeg";
        commands[1] = "-rtsp_transport";
        commands[2] = "tcp";
        commands[3] = "-i";
        commands[4] = inputPath;
        commands[5] = "-vcodec";
        commands[6] = "copy";
        commands[7] = "-acodec";
        commands[8] = "aac";
        commands[9] = "-f";
        commands[10] = "mp4";
        commands[11] = "-y";//覆盖
        commands[12] = outputPath;

        return commands;
    }




}
