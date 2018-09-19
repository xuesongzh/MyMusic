package com.zxsong.media.myplayer.player;


import android.text.TextUtils;
import android.util.Log;

import com.zxsong.media.myplayer.listener.OnPreparedListener;

public class XsPlayer {

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("swscale-4");
        System.loadLibrary("postproc-54");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avdevice-57");
    }

    private static final String TAG = "XsPlayer";

    //数据源
    private String source;

    private OnPreparedListener mOnPreparedListener;

    public XsPlayer() {

    }

    /**
     * 设置数据源
     * @param source
     */
    public void setSource(String source) {
        this.source = source;
    }

    /**
     * 设置准备接口回调
     * @param onPreparedListener
     */
    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.mOnPreparedListener = onPreparedListener;
    }

    public void prepared() {
        if (TextUtils.isEmpty(source)) {
            Log.d(TAG, "source is empty !");
            return;
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
                n_prepared(source);
            }
        }).start();
    }

    public void start() {
        if (TextUtils.isEmpty(source)) {
            Log.d(TAG, "source is empty !");
            return;
        }

        new Thread(new Runnable() {
            @Override
            public void run() {
                n_start();
            }
        }).start();
    }

    /**
     * c++回调java的方法
     */
    public void onCallPrepared() {
        if (mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared();
        }
    }

    public native void n_prepared(String source);
    public native void n_start();

}
