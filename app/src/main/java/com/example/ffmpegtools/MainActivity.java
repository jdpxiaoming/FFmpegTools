package com.example.ffmpegtools;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.content.PermissionChecker;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import com.jdpxiaoming.ffmpeg_cmd.FFmpegCmd;
import com.jdpxiaoming.ffmpeg_cmd.FFmpegFactory;
import com.jdpxiaoming.ffmpeg_cmd.FFmpegUtil;
import java.io.File;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    private int requestPermissionCode = 10086;
    private String[] requestPermission = new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // Example of a call to a native method
        // Example of a call to a native method
        if(Build.VERSION.SDK_INT > Build.VERSION_CODES.M){
            if(PermissionChecker.checkSelfPermission(this,Manifest.permission.WRITE_EXTERNAL_STORAGE) != PermissionChecker.PERMISSION_GRANTED){
                requestPermissions(requestPermission,requestPermissionCode);
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case 1001:
                // 1001的请求码对应的是申请打电话的权限
                // 判断是否同意授权，PERMISSION_GRANTED 这个值代表的是已经获取了权限
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(MainActivity.this, "你同意授权了", Toast.LENGTH_LONG).show();
                } else {
                    Toast.makeText(MainActivity.this, "你不同意授权", Toast.LENGTH_LONG).show();
                }
                break;
        }
    }

    /**
     * 视频转码 flv->mp4.
     * @param view
     */
    public void videoTransform(View view) {
        String inputPath = Environment.getExternalStorageDirectory().getAbsolutePath()+"/Download/58.flv";
        String outputPath = Environment.getExternalStorageDirectory().getAbsolutePath()+"/Download/62.mp4";
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
//        inputPath ="http://106.14.218.234:5581/rtsp/0d427a62-3f7b-44e6-b81f-e891ba79f994/live.flv";
        inputPath = "http://101.133.158.71:5581/rtsp/942f4c4e-377d-4581-9812-e7306cfc3a36/live.flv";

        String[] commands = FFmpegFactory.buildFlv2Mp4(inputPath,outputPath);

        FFmpegUtil.getInstance().enQueueTask(commands, 0,new FFmpegUtil.onCallBack() {
            @Override
            public void onStart() {

                Log.i(TAG," onStart # ");
            }

            @Override
            public void onFailure() {
                Log.i(TAG," onFailure # ");
                Toast.makeText(MainActivity.this, "transcode failed ,please check your input file !", Toast.LENGTH_LONG).show();
            }

            @Override
            public void onComplete() {
                Log.i(TAG," onComplete # ");
                Toast.makeText(MainActivity.this, "transcode successful!", Toast.LENGTH_LONG).show();
            }

            @Override
            public void onProgress(float progress) {
                Log.i(TAG," onProgress # "+progress);
            }
        });

        outputPath = Environment.getExternalStorageDirectory().getAbsolutePath()+"/Download/63.mp4";
        output =new File(outputPath);
        if(output.exists()){
            output.delete();
        }
        commands = FFmpegFactory.buildFlv2Mp4(inputPath,outputPath);

        FFmpegUtil.getInstance().enQueueTask(commands, 0,new FFmpegUtil.onCallBack() {
            @Override
            public void onStart() {

                Log.i(TAG," onStart2 # ");
            }

            @Override
            public void onFailure() {
                Log.i(TAG," onFailure2 # ");
                Toast.makeText(MainActivity.this, "transcode failed2 ,please check your input file2 !", Toast.LENGTH_LONG).show();
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

    }

    /**
     * 停止命令.
     * @param view
     */
    public void stopRun(View view) {
        FFmpegUtil.getInstance().stopTask();
    }

    public void dumpFlv(View view) {

        //flv测试ok.
        String input = "http://118.31.174.18:5581/rtmp/8e5196c4-e7d9-41b0-9080-fa0da638d9e2/live.flv";//flv测试流.
        //1. h265+pcma 测试failed 猜测是音频格式pcma无法封装成mp4格式.
        input = "rtsp://47.108.158.50:5555/rtsp/9bf29ca9-e26e-462a-bb0e-3e08461b91e8";//rtsp 测试流
        //2. h264+aac 测试ok.
        input = "http://47.108.158.50:6681/rtsp/a2259dbb-301a-47d8-ac37-b372684153f0/index.m3u8";

        String output = new File(Environment.getExternalStorageDirectory(),"/poe/output63.mp4").getAbsolutePath();
        FFmpegCmd.dump_stream(input , output);
    }
}
