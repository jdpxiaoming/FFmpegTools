package com.example.ffmpegtools;

import android.util.Log;

/**
 * ffmpeg 工具类.
 * @author jdpxiaoming 2020/05/16
 */
public class FFmpegCmd
{

    private static final String TAG ="FFmpegCmd";

    static
    {
        System.loadLibrary("ffmpeg-cmd");
    }

    private static OnCmdExecListener sOnCmdExecListener;
    private static long sDuration;

    public static native int exec(int argc, String[] argv);

    public static native void exit();

    public static void exec(String[] cmds, long duration, OnCmdExecListener listener) {
        sOnCmdExecListener = listener;
        sDuration = duration;

        exec(cmds.length, cmds);
    }

    public static void exec(String[] cmds, OnCmdExecListener listener) {
        sOnCmdExecListener = listener;
        exec(cmds.length, cmds);
    }


    public static void onExecuted(int ret)
    {
        Log.i(TAG," onExecuted # "+ret);
        if (sOnCmdExecListener != null)
        {
            if (ret == 0)
            {
                sOnCmdExecListener.onProgress(sDuration);
                sOnCmdExecListener.onSuccess();
            }
            else
            {
                sOnCmdExecListener.onFailure();
            }
        }
    }

    /**
     * 转码进度回调. .
     * jni 会回调此方法 .
     * @param progress
     */
    public static void onProgress(float progress)
    {
        Log.i(TAG," onProgress # "+progress);
        if (sOnCmdExecListener != null){
            if (sDuration != 0){
                sOnCmdExecListener.onProgress(progress / (sDuration / 1000) * 0.95f);
            }
        }
    }

    /**
     * 动作结束回调，或者出错中断了 .
     * jni调用java方法通知：任务完成 .
     */
    public static void onComplete(){
        Log.i(TAG," onComplete ()# action done!");
        if (sOnCmdExecListener != null){
            sOnCmdExecListener.onComplete();
        }
    }


    public interface OnCmdExecListener {
        void onSuccess();

        void onFailure();

        void onComplete();

        void onProgress(float progress);
    }

}
