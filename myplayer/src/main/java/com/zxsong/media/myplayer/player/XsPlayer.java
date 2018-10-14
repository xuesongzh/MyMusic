package com.zxsong.media.myplayer.player;


import android.text.TextUtils;
import android.util.Log;

import com.zxsong.media.myplayer.bean.TimeInfoBean;
import com.zxsong.media.myplayer.listener.OnLoadListener;
import com.zxsong.media.myplayer.listener.OnPauseResumeListener;
import com.zxsong.media.myplayer.listener.OnPreparedListener;
import com.zxsong.media.myplayer.listener.OnTimeInfoListener;

public class XsPlayer {

    private static final String TAG = "XsPlayer";

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

    //数据源
    private String source;
    private static TimeInfoBean sTimeInfoBean;

    private OnPreparedListener mOnPreparedListener;
    private OnLoadListener mOnLoadListener;
    private OnPauseResumeListener mOnPauseResumeListener;
    private OnTimeInfoListener mOnTimeInfoListener;

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

    public void setOnLoadListener(OnLoadListener onLoadListener) {
        this.mOnLoadListener = onLoadListener;
    }

    public void setOnPauseResumeListener(OnPauseResumeListener onPauseResumeListener) {
        this.mOnPauseResumeListener = onPauseResumeListener;
    }

    public void setOnTimeInfoListener(OnTimeInfoListener onTimeInfoListener) {
        mOnTimeInfoListener = onTimeInfoListener;
    }

    public void prepared() {
        if (TextUtils.isEmpty(source)) {
            Log.d(TAG, "source is empty !");
            return;
        }

        onCallLoad(true);
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

    public void pause() {
        n_pause();
        if (mOnPauseResumeListener != null) {
            mOnPauseResumeListener.onPause(true);
        }
    }

    public void resume() {
        n_resume();
        if (mOnPauseResumeListener != null) {
            mOnPauseResumeListener.onPause(false);
        }
    }
    /**
     * c++回调java的方法
     */
    public void onCallPrepared() {
        if (mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared();
        }
    }

    public void onCallLoad(boolean load) {
        if (mOnLoadListener != null) {
            mOnLoadListener.onLoad(load);
        }
    }

    public void onCallTimeInfo(int currentTime, int totalTime) {
        if (mOnTimeInfoListener != null) {
            if (sTimeInfoBean == null) {
                sTimeInfoBean = new TimeInfoBean();
            }
            sTimeInfoBean.setCurrentTime(currentTime);
            sTimeInfoBean.setTotalTime(totalTime);
            mOnTimeInfoListener.onTimeInfo(sTimeInfoBean);
        }
    }

    private native void n_prepared(String source);
    private native void n_start();
    private native void n_pause();
    private native void n_resume();
}
