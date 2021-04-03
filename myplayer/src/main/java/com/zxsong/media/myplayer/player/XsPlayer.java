package com.zxsong.media.myplayer.player;


import android.text.TextUtils;
import android.util.Log;

import com.zxsong.media.myplayer.bean.TimeInfoBean;
import com.zxsong.media.myplayer.listener.OnCompleteListener;
import com.zxsong.media.myplayer.listener.OnErrorListener;
import com.zxsong.media.myplayer.listener.OnLoadListener;
import com.zxsong.media.myplayer.listener.OnPauseResumeListener;
import com.zxsong.media.myplayer.listener.OnPreparedListener;
import com.zxsong.media.myplayer.listener.OnTimeInfoListener;
import com.zxsong.media.myplayer.opengl.XsGLSurfaceView;

public class XsPlayer {

    private static final String TAG = "XsPlayer";

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("avcodec");
        System.loadLibrary("avformat");
        System.loadLibrary("swscale");
        System.loadLibrary("postproc");
        System.loadLibrary("avfilter");
        System.loadLibrary("avdevice");
    }

    //数据源
    private String source;
    private static TimeInfoBean sTimeInfoBean;
    private boolean playNext = false;
    private int duration;

    private OnPreparedListener mOnPreparedListener;
    private OnLoadListener mOnLoadListener;
    private OnPauseResumeListener mOnPauseResumeListener;
    private OnTimeInfoListener mOnTimeInfoListener;
    private OnErrorListener mOnErrorListener;
    private OnCompleteListener mOnCompleteListener;
    private XsGLSurfaceView mXsGLSurfaceView;

    public XsPlayer() {

    }

    /**
     * 设置数据源
     *
     * @param source
     */
    public void setSource(String source) {
        this.source = source;
    }

    public void setXsGLSurfaceView(XsGLSurfaceView xsGLSurfaceView) {
        mXsGLSurfaceView = xsGLSurfaceView;
    }

    public int getDuration() {
       return duration;
    }

    /**
     * 设置准备接口回调
     *
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

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        mOnErrorListener = onErrorListener;
    }

    public void setOnCompleteListener(OnCompleteListener onCompleteListener) {
        mOnCompleteListener = onCompleteListener;
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

    public void stop() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
            }
        }).start();
    }

    public void seek(int seconds) {
        n_seek(seconds);
    }

    public void playNext(String url) {
        source = url;
        playNext = true;
        stop();
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
            duration = totalTime;
            mOnTimeInfoListener.onTimeInfo(sTimeInfoBean);
        }
    }

    public void onCallError(int code, String msg) {
        if (mOnErrorListener != null) {
            stop();
            mOnErrorListener.onError(code, msg);
        }
    }

    public void onCallComplete() {
        if (mOnCompleteListener != null) {
            stop();
            mOnCompleteListener.onComplete();
        }
    }

    public void onCallNext() {
        if (playNext) {
            playNext = false;
            prepared();
        }
    }

    public void onCallRenderYUV(int width, int height, byte[] y, byte[] u, byte[] v) {
        Log.d(TAG,"获取到视频的yuv数据");
        if (mXsGLSurfaceView != null) {
//            mXsGLSurfaceView.getWlRender().setRenderType(WlRender.RENDER_YUV);
            mXsGLSurfaceView.setYUVData(width, height, y, u, v);
        }
    }

    private native void n_prepared(String source);

    private native void n_start();

    private native void n_pause();

    private native void n_resume();

    private native void n_stop();

    private native void n_seek(int seconds);

}
