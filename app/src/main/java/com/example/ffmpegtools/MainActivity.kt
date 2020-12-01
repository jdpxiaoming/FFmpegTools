package com.example.ffmpegtools

import android.Manifest
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.example.ffmpegtools.R
import android.os.Build
import androidx.core.content.PermissionChecker
import android.content.pm.PackageManager
import android.os.Environment
import android.util.Log
import android.view.View
import android.widget.Toast
import com.jdpxiaoming.ffmpeg_cmd.FFmpegFactory
import com.jdpxiaoming.ffmpeg_cmd.FFmpegUtil
import com.jdpxiaoming.ffmpeg_cmd.FFmpegUtil.onCallBack
import com.example.ffmpegtools.MainActivity
import com.jdpxiaoming.ffmpeg_cmd.FFmpegCmd
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File

/**
 * this is a demo for how to use ffmpegtools
 * https://github.com/jdpxiaoming/FFmpegTools
 *
 * @author jdpxiaoming 2020/12/01
 */
class MainActivity : AppCompatActivity() {

    companion object {
        private const val TAG = "MainActivity"
    }


    private val requestPermissionCode = 10086
    private val requestPermission = arrayOf(Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        // Example of a call to a native method
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.M) {
            if (PermissionChecker.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PermissionChecker.PERMISSION_GRANTED) {
                requestPermissions(requestPermission, requestPermissionCode)
            }
        }

        initViews();
    }



    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            1001 ->                 // 1001的请求码对应的是申请打电话的权限
                // 判断是否同意授权，PERMISSION_GRANTED 这个值代表的是已经获取了权限
                if (grantResults.size > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(this@MainActivity, "你同意授权了", Toast.LENGTH_LONG).show()
                } else {
                    Toast.makeText(this@MainActivity, "你不同意授权", Toast.LENGTH_LONG).show()
                }
        }
    }

    /**
     * 视频转码 flv->mp4.
     * @param view
     */
    fun videoTransform(view: View?) {
        val inputPath = "http://113.31.102.114:5581/rtsp/9c137ab8-dde5-4342-9aa0-6ad29a862a9a.flv"
        val outputPath = Environment.getExternalStorageDirectory().absolutePath + "/Download/1117.mp4"
        val output = File(outputPath)
        if (output.exists()) {
            output.delete()
        }
        //cmds for ffmpeg flv->mp4.
        var commands: Array<String?>? = null // FFmpegFactory.buildRtsp2Mp4(inputPath,outputPath);
        commands = FFmpegFactory.buildFlv2Mp4(inputPath, outputPath)
        FFmpegUtil.getInstance().enQueueTask(commands, 0, object : onCallBack {
            override fun onStart() {
                Log.i(TAG, " onStart2 # ")
            }

            override fun onFailure() {
                Log.i(TAG, " onFailure2 # ")
                Toast.makeText(this@MainActivity, "transcode failed2 ,please check your input file2 !", Toast.LENGTH_LONG).show()
            }

            override fun onComplete() {
                Log.i(TAG, " onComplete2 # ")
                Toast.makeText(this@MainActivity, "transcode successful2!", Toast.LENGTH_LONG).show()
            }

            override fun onProgress(progress: Float) {
                Log.i(TAG, " onProgress2 # $progress")
            }
        })
    }


    /**
     * 初始化urls.
     */
    private fun initViews() {
        val inputPath = "http://113.31.102.114:5581/rtsp/9c137ab8-dde5-4342-9aa0-6ad29a862a9a.flv"
        val outputPath = Environment.getExternalStorageDirectory().absolutePath + "/Download/1117.mp4"
        val output = File(outputPath)
        if (output.exists()) {
            output.delete()
        }
        //cmds for ffmpeg flv->mp4.
        var commands: Array<String?>? = null // FFmpegFactory.buildRtsp2Mp4(inputPath,outputPath);
        commands = FFmpegFactory.buildFlv2Mp4(inputPath, outputPath)

        commands?.let {
            var jpsb:StringBuffer = StringBuffer();
            for(str in commands){
                jpsb.append("$str ")
            }

            cmd_et.setText(jpsb.toString());
        }

        //转化flv的地址.
        flv_et.setText("http://106.75.254.198:5581/rtsp/e075fedc-ca6d-46ee-b40c-55500990ee50.flv");
        //转化rtsp（hevc)的地址
        rtsp_et.setText("rtsp://47.108.81.159:5555/rtsp/e8f98226-5263-472c-8bbc-e3ec06c7ab1d");
    }


    /**
     * 停止命令.
     * @param view
     */
    fun stopRun(view: View?) {
        FFmpegUtil.getInstance().stopTask()
    }

    /**
     * flv或下载保存为mp4文件 .
     * @param view
     */
    fun dumpFlv(view: View?) {
        Log.i(TAG, "Java#dumpFlv~start~")
        //flv测试ok.
//        String input = "rtsp://47.108.81.159:5555/rtsp/e8f98226-5263-472c-8bbc-e3ec06c7ab1d";
        val input = flv_et.text.toString();
        val output = File(Environment.getExternalStorageDirectory(), "/poe/output63.mp4").absolutePath
        FFmpegCmd.dump_stream(input, output)
    }

    /**
     * 下载h264保存为本地mp4文件.
     * @param view
     */
    fun dumpRtspToFlv(view: View?) {
        Log.i(TAG, "Java#dumpRtspToFlv~start~")
        //flv测试ok.
        val input = rtsp_et.text.toString();
        val output = File(Environment.getExternalStorageDirectory(), "/poe/output73.mp4").absolutePath
        FFmpegCmd.dump_stream(input, output)
    }
}