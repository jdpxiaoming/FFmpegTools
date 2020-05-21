package com.example.ffmpegtools;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import android.Manifest;
import android.content.pm.PackageManager;
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

    private String[] mPermission =  new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // Example of a call to a native method
        if(ContextCompat.checkSelfPermission(this,Manifest.permission.WRITE_EXTERNAL_STORAGE)
        != PackageManager.PERMISSION_GRANTED||
                ContextCompat.checkSelfPermission(this,Manifest.permission.READ_EXTERNAL_STORAGE)
                        != PackageManager.PERMISSION_GRANTED){
            ActivityCompat.requestPermissions(this, mPermission, 1001);
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
        String[] commands = FFmpegFactory.buildFlv2Mp4(inputPath,outputPath);

        FFmpegUtil.getInstance().enQueueTask(commands, 0,new FFmpegUtil.onCallBack() {
            @Override
            public void onStart() {

                Log.i(TAG," onStart # ");
            }

            @Override
            public void onFailure() {
                Log.i(TAG," onFailure # ");
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
}
