package com.example.ffmpegtools;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("ffmpeg-cmd");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText("hello world");

        tv.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                videoTransform(v);
            }
        });


        if (Build.VERSION.SDK_INT >= 23) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
        }
    }



    /**
     * 视频转码 flv->mp4.
     * @param view
     */
    public void videoTransform(View view) {
        final String inputPath = getCacheDir().getAbsolutePath()+"/58.flv";
//        final String outputPath = getCacheDir().getAbsolutePath()+"/59.mp4";
        final String outputPath = Environment.getExternalStorageDirectory().getAbsolutePath()+"/Download/60.mp4";
        File output =new File(outputPath);
        if(output.exists()){
            output.delete();
        }
        final String[] commands = new String[8];
        commands[0] = "ffmpeg";
        commands[1] = "-i";
        commands[2] = inputPath;
        commands[3] = "-vcodec";
        commands[4] = "copy";
        commands[5] = "-acodec";
        commands[6] = "aac";
//        commands[7] = "-movflags";
//        commands[8] = " +faststart";
        commands[7] = outputPath;

        exec(commands.length,commands);
    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */

    public static native int exec(int argc, String[] argv);

    public static native void exit();
}
