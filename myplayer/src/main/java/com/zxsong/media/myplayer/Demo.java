package com.zxsong.media.myplayer;

/**
 * Created by zhangxuesong on 2018/7/11.
 */

public class Demo {
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public native String stringFromJNI();
}
