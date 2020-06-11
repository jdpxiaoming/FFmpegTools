package com.jdpxiaoming.ffmpeg_cmd;

/**
 * ffmpeg tools.
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
    private static long sDuration = 0 ;

    public static native int exec(int argc, String[] argv);

    public static native void exit();

    public static  native int dump_stream(String input, String output);

    /**
     * this method invoked child thread , please use {@link FFmpegUtil#exec(String[], FFmpegUtil.onCallBack)}
     * @param cmds
     * @param duration
     * @param listener
     */
    public static void exec(String[] cmds, long duration, OnCmdExecListener listener) {
        sOnCmdExecListener = listener;
        sDuration = duration;

        exec(cmds.length, cmds);
    }

    /**
     * this method invoked child thread , please use {@link FFmpegUtil#exec(String[], FFmpegUtil.onCallBack)}
     * @param cmds
     * @param listener
     */
    public static void exec(String[] cmds, OnCmdExecListener listener) {
        sOnCmdExecListener = listener;
        exec(cmds.length, cmds);
    }


    public static void onExecuted(int ret)
    {
        FLog.i(TAG," onExecuted # "+ret);
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
     * transcode on progress .
     * jni invoked this method .
     * @param progress
     */
    public static void onProgress(float progress)
    {
        FLog.i(TAG," onProgress # "+progress);
        if (sOnCmdExecListener != null){
            if (sDuration != 0){
                sOnCmdExecListener.onProgress(progress / (sDuration / 1000) * 0.95f);
            }else{
                sOnCmdExecListener.onProgress(progress);
            }
        }
    }

    /**
     *  task finish invoked.
     * jni invoked this method when task is finished.
     */
    public static void onComplete(){
        FLog.i(TAG," onComplete ()# action done!");
        if (sOnCmdExecListener != null){
            sOnCmdExecListener.onComplete();
        }
    }


    /**
     *  task finish invoked.
     * jni invoked this method when task is finished.
     */
    public static void onFailure(){
        FLog.i(TAG," onFailure ()# action done!");
        if (sOnCmdExecListener != null){
            sOnCmdExecListener.onFailure();
        }
    }

    /**
     * stop下载任务回调.
     */
    public static void onCancelFinish(){
        FLog.i(TAG," onCancelFinish ()# action done!");
        if (sOnCmdExecListener != null){
            sOnCmdExecListener.onCancelFinish();
        }
    }

    public interface OnCmdExecListener {
        void onSuccess();

        void onFailure();

        void onComplete();

        void onProgress(float progress);

        /**
         * 取消完成.
         */
        void onCancelFinish();
    }

}
