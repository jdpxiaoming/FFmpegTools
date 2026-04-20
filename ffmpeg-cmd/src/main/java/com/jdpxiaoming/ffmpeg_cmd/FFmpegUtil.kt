/*
 *
 *  * @author jdpxiaoming
 *  * @github https://github.com/jdpxiaoming
 *  * created  20-5-20 下午4:53: $time
 */

package com.jdpxiaoming.ffmpeg_cmd;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.util.Log;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

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
    private static final int MSG_ON_CANCEL_FINISH = 0x005;
    private static final int MSG_ON_ERROR = 0x101;

    private Handler mHandler;

    private onCallBack mCallbackListener;//回调.

    // TODO: 2020/5/21 缓存请求命令到一个缓存队列，按序执行 .
    private LinkedBlockingQueue<FFmepgTask> mAsynTaskQueue = new LinkedBlockingQueue<>();

    //固定一个线程来实现异步，方便后期扩展.
    private ExecutorService mThreadPoolService = Executors.newFixedThreadPool(1);

    private boolean isRunning = false;

    private FFmepgTask mCurrentTask = null;

    private Object mLock = null;


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

    private FFmpegUtil(){

        if(null == mHandler){
            mHandler = new Handler(Looper.getMainLooper(),this);
        }
        mLock = new Object();
        // TODO: 2020/5/21 开启读取线程.
        isRunning =true;
        mThreadPoolService.execute(mReadThread);
    }


    private Runnable mReadThread = new Runnable() {
        @Override
        public void run() {

            for(;;){

                if(!isRunning) {
                    FLog.e(TAG,"mReadThread stop isRunning is false !");
                    break;
                }

                try {
                    synchronized (mLock){
                      //没有让任务等待，拿到任务继续执行
                      mCurrentTask =  mAsynTaskQueue.take();
                      if(null != mCurrentTask){
                          //枷锁，等待当前任务执行完毕.MSG_ON_COMPLETE被执行.
                          exec(mCurrentTask.getCmds(),mCurrentTask.getDuration(),mCurrentTask.callBacklistener);
                      }
                      mLock.wait();
                   }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    };




    /**
     * 结束当前任务.清空等待队列. 结束后台任务.
     */
    public void stopTask(){
        isRunning = false;
        mAsynTaskQueue.clear();
        synchronized (mLock){
            mLock.notifyAll();
        }
        FFmpegCmd.exit();
    }

    /**
     * untest please do not use this .
     */
    public void onDestroy(){
        isRunning = false;
        mThreadPoolService.shutdown();
        try {
            if(!mThreadPoolService.awaitTermination(100, TimeUnit.MILLISECONDS)){
                mThreadPoolService.shutdownNow();
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    /**
     * 同步执行任务。 先进先出.
     * @param cmds
     * @param duration
     * @param listener
     */
    public void enQueueTask(String[] cmds, long duration, onCallBack listener){
        Log.e(TAG,"enQueueTask()");

        long id = SystemClock.currentThreadTimeMillis();
        FFmepgTask task = new FFmepgTask(id, duration,cmds,listener);
        mAsynTaskQueue.add(task);
        if(!isRunning){
            isRunning =true;
            mThreadPoolService.execute(mReadThread);
        }
    }


    /**
     * 执行底层命令 .
     * @param cmds string[] commands collection . {@link FFmpegFactory#buildFlv2Mp4(String, String)}
     * @param duration total time long in millisecond .
     * @param listener listen callback .
     */
    private void exec(String[] cmds, long duration, onCallBack listener) {
        FLog.e(TAG,"exec() ~FFmpegCmd.exec..... !");
        mCallbackListener = listener;

        FFmpegCmd.exec(cmds, duration, new FFmpegCmd.OnCmdExecListener() {
            @Override
            public void onSuccess() {
                FLog.i(TAG," onSuccess # ");
                if(null != mHandler) mHandler.sendEmptyMessage(MSG_ON_START);
            }

            @Override
            public void onFailure() {
                FLog.e(TAG," onFailure # ");
                if(null != mHandler) mHandler.sendEmptyMessage(MSG_ON_FAILURE);

            }

            @Override
            public void onComplete() {
                FLog.e(TAG," onComplete #mHandler~ ");
                if(null != mHandler) mHandler.sendEmptyMessage(MSG_ON_COMPLETE);
            }

            @Override
            public void onProgress(float progress) {
                FLog.i(TAG," onProgress # ");
                if(null != mHandler) mHandler.sendMessage(mHandler.obtainMessage(MSG_ON_PROGRESS,progress));
            }

            @Override
            public void onCancelFinish() {
                FLog.e(TAG," onCancelFinish #mHandler~ ");
                if(null != mHandler) mHandler.sendEmptyMessage(MSG_ON_CANCEL_FINISH);
            }
        });
    }

    private void exec(String[] cmds, onCallBack listener) {
        exec(cmds, 0 ,listener );
    }


    @Override
    public boolean handleMessage(Message msg) {

        switch (msg.what){
            default:
            case MSG_ON_START://开始执行
                FLog.i(TAG," MSG_ON_START # ");
                if(null != mCallbackListener) mCallbackListener.onStart();
                break;
            case MSG_ON_FAILURE://fail
                FLog.i(TAG," MSG_ON_FAILURE # ");
                synchronized (mLock){
                    if(null != mCallbackListener) mCallbackListener.onFailure();
                    mLock.notifyAll();
                }
                break;
            case MSG_ON_PROGRESS://progress .
                //视频时长单位s(秒).
                float currentTime = (float) msg.obj;
                FLog.i(TAG," onProgress # "+currentTime);
                if(null != mCallbackListener) mCallbackListener.onProgress(currentTime);
                break;
            case MSG_ON_COMPLETE://finish .
                FLog.i(TAG," MSG_ON_COMPLETE # ");
                // DO: 2020/5/21 通知任务继续执行.
                synchronized (mLock){
                    if(null != mCallbackListener) mCallbackListener.onComplete();
                    mLock.notifyAll();
                }
                break;
            case MSG_ON_CANCEL_FINISH://cancel finish .
                FLog.i(TAG," MSG_ON_COMPLETE # ");
                synchronized (mLock){
                    mLock.notifyAll();
                }
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
