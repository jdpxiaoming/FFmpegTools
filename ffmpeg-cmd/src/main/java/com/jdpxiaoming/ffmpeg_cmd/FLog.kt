/*
 *
 *  * @author jdpxiaoming
 *  * @github https://github.com/jdpxiaoming
 *  * created  20-5-21 上午9:02: $time
 */
package com.jdpxiaoming.ffmpeg_cmd;

import android.util.Log;


public class FLog {

    public static void i(String tag , String args){
//        if(BuildConfig.DEBUG){
            Log.i(tag,args);
//        }
    }


    public static void e(String tag , String args){
//        if(BuildConfig.DEBUG){
            Log.e(tag,args);
//        }
    }
}
