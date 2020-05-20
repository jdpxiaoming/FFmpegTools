/*
 *
 *  * @author jdpxiaoming
 *  * @github https://github.com/jdpxiaoming
 *  * created  20-5-20 下午4:53: $time
 */

package com.jdpxiaoming.ffmpeg_cmd;

import android.os.Handler;
import android.os.Message;

import androidx.annotation.NonNull;

/**
 * ffmepg tools thread attack to main thread util class.
 * @author jdpxiaoming
 * @github https://github.com/jdpxiaoming
 * created  2020/05/20 16:48
 */
public class FFmpegUtil implements Handler.Callback {

    private static final String TAG = "FFmpegUtil";

    private static volatile FFmpegUtil mInstance;

    private static final int MSG_ON_START = 0x001;
    private static final int MSG_ON_FAILURE = 0x002;
    private static final int MSG_ON_PROGRESS = 0x003;
    private static final int MSG_ON_COMPLETE = 0x004;
    private static final int MSG_ON_ERROR = 0x101;

    private Handler mHandler = new Handler(this);

    private onCallBack mCallbackListener;//回调.


    public static FFmpegUtil getInstance(){

        if(null == mInstance){

            synchronized (FFmpegUtil.class){

                if(null == mInstance){

                    mInstance = new FFmpegUtil();
                }
            }
        }
        return mInstance;
    }

    private FFmpegUtil(){}


    /**
     * 执行底层命令 .
     * @param cmds string[] commands collection . {@link FFmpegFactory#buildFlv2Mp4(String, String)}
     * @param duration total time long in millisecond .
     * @param listener listen callback .
     */
    public void exec(String[] cmds, long duration, onCallBack listener) {
        mCallbackListener = listener;

        FFmpegCmd.exec(cmds, duration, new FFmpegCmd.OnCmdExecListener() {
            @Override
            public void onSuccess() {
                if(null != mHandler) mHandler.sendEmptyMessage(MSG_ON_START);
            }

            @Override
            public void onFailure() {
                if(null != mHandler) mHandler.sendEmptyMessage(MSG_ON_FAILURE);

            }

            @Override
            public void onComplete() {
                if(null != mHandler) mHandler.sendEmptyMessage(MSG_ON_COMPLETE);
            }

            @Override
            public void onProgress(float progress) {
                if(null != mHandler) mHandler.sendMessage(mHandler.obtainMessage(MSG_ON_PROGRESS,progress));
            }
        });
    }

    public void exec(String[] cmds, onCallBack listener) {
        exec(cmds, 0 ,listener );
    }


    @Override
    public boolean handleMessage(@NonNull Message msg) {

        switch (msg.what){
            default:
            case MSG_ON_START://开始执行
                if(null != mCallbackListener) mCallbackListener.onStart();
                break;
            case MSG_ON_FAILURE://fail
                if(null != mCallbackListener) mCallbackListener.onFailure();
                break;
            case MSG_ON_PROGRESS://progress .
                //视频时长单位s(秒).
                float currentTime = (float) msg.obj;

                if(null != mCallbackListener) mCallbackListener.onProgress(currentTime);
                break;
            case MSG_ON_COMPLETE://finish .
                if(null != mCallbackListener) mCallbackListener.onComplete();
                break;
        }
        return true;
    }



    public interface onCallBack {

        /**
         * 成功执行代码.
         */
        void onStart();

        /**
         * 中断错误.
         */
        void onFailure();

        /**
         * 执行任务完成.
         */
        void onComplete();

        /**
         * 进度回掉.
         * @param progress
         */
        void onProgress(float progress);
    }


}
