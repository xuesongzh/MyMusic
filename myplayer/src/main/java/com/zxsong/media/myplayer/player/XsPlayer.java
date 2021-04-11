package com.zxsong.media.myplayer.player;


import android.media.MediaCodec;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import com.zxsong.media.myplayer.bean.TimeInfoBean;
import com.zxsong.media.myplayer.listener.OnCompleteListener;
import com.zxsong.media.myplayer.listener.OnErrorListener;
import com.zxsong.media.myplayer.listener.OnLoadListener;
import com.zxsong.media.myplayer.listener.OnPauseResumeListener;
import com.zxsong.media.myplayer.listener.OnPreparedListener;
import com.zxsong.media.myplayer.listener.OnTimeInfoListener;
import com.zxsong.media.myplayer.opengl.XsGLSurfaceView;
import com.zxsong.media.myplayer.opengl.XsRender;
import com.zxsong.media.myplayer.util.XsVideoSupportUtil;

import java.nio.ByteBuffer;

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

    private MediaFormat mMediaFormat;
    private MediaCodec mMediaCodec;
    private Surface mSurface;
    private MediaCodec.BufferInfo mBufferInfo;

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
        mXsGLSurfaceView.getRender().setOnSurfaceCreateListener(new XsRender.OnSurfaceCreateListener() {
            @Override
            public void onSurfaceCreate(Surface surface) {
                mSurface = surface;
            }
        });
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
        sTimeInfoBean = null;
        duration = 0;
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
                releaseMediaCodec();
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
        Log.d(TAG, "获取到视频的yuv数据");
        if (mXsGLSurfaceView != null) {
            mXsGLSurfaceView.getRender().setRenderType(XsRender.RENDER_YUV);
            mXsGLSurfaceView.setYUVData(width, height, y, u, v);
        }
    }


    public boolean onCallIsSupportMediaCodec(String codecName) {
        return XsVideoSupportUtil.isSupportCodec(codecName);
    }

    public void initMediaCodec(String codecName, int width, int height, byte[] csd_0, byte[] csd_1) {
        if (mSurface != null) {
            try {
                mXsGLSurfaceView.getRender().setRenderType(XsRender.RENDER_MEDIA_CODEC);
                String mime = XsVideoSupportUtil.findVideoCodecName(codecName);
                mMediaFormat = MediaFormat.createVideoFormat(mime, width, height);
                mMediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                mMediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(csd_0));
                mMediaFormat.setByteBuffer("csd-1", ByteBuffer.wrap(csd_1));
                Log.d(TAG, mMediaFormat.toString());

                mMediaCodec = MediaCodec.createDecoderByType(mime);
                mBufferInfo = new MediaCodec.BufferInfo();
                mMediaCodec.configure(mMediaFormat, mSurface, null, 0);
                mMediaCodec.start();

            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            if (mOnErrorListener != null) {
                mOnErrorListener.onError(2001, "surface is null");
            }
        }
    }

    public void decodeAVPacket(byte[] data, int dataSize) {
        if (mSurface != null && data != null && dataSize > 0 && mMediaCodec != null) {
            try {
                int inputBufferIndex = mMediaCodec.dequeueInputBuffer(10);
                if (inputBufferIndex >= 0) {
                    ByteBuffer byteBuffer = mMediaCodec.getInputBuffers()[inputBufferIndex];
                    byteBuffer.clear();
                    byteBuffer.put(data);
                    // 填充数据
                    mMediaCodec.queueInputBuffer(inputBufferIndex, 0, dataSize, 0, 0);
                }
                int outputBufferIndex = mMediaCodec.dequeueOutputBuffer(mBufferInfo, 10);
                while (outputBufferIndex >= 0) {
                    mMediaCodec.releaseOutputBuffer(outputBufferIndex, true);
                    outputBufferIndex = mMediaCodec.dequeueOutputBuffer(mBufferInfo, 10);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }

        }
    }

    private void releaseMediaCodec() {
        if (mMediaCodec != null) {
            try {
                mMediaCodec.flush();
                mMediaCodec.stop();
                mMediaCodec.release();
            } catch (Exception e) {
                e.printStackTrace();
            }
            mMediaCodec = null;
            mMediaFormat = null;
            mBufferInfo = null;
        }

    }


    private native void n_prepared(String source);

    private native void n_start();

    private native void n_pause();

    private native void n_resume();

    private native void n_stop();

    private native void n_seek(int seconds);

}
