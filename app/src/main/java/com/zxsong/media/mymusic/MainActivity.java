package com.zxsong.media.mymusic;

import android.Manifest;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.zxsong.media.mymusic.utils.PermissionUtils;
import com.zxsong.media.mymusic.utils.TimeUtils;
import com.zxsong.media.myplayer.bean.TimeInfoBean;
import com.zxsong.media.myplayer.listener.OnCompleteListener;
import com.zxsong.media.myplayer.listener.OnErrorListener;
import com.zxsong.media.myplayer.listener.OnLoadListener;
import com.zxsong.media.myplayer.listener.OnPauseResumeListener;
import com.zxsong.media.myplayer.listener.OnPreparedListener;
import com.zxsong.media.myplayer.listener.OnTimeInfoListener;
import com.zxsong.media.myplayer.opengl.XsGLSurfaceView;
import com.zxsong.media.myplayer.player.XsPlayer;

public class MainActivity extends AppCompatActivity {

    private XsPlayer mXsPlayer;
    private TextView mTextView;
    private XsGLSurfaceView mXsGLSurfaceView;
    private SeekBar mSeekBar;
    private int position;
    private boolean seek = false;

    private static final String TAG = "MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        PermissionUtils.requestPermissions(this, 1, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE},
                new PermissionUtils.OnPermissionListener() {
                    @Override
                    public void onPermissionGranted() {

                    }

                    @Override
                    public void onPermissionDenied(String[] deniedPermissions, boolean alwaysDenied) {

                    }
                });
        mTextView = findViewById(R.id.tv_time);
        mXsGLSurfaceView = findViewById(R.id.xsglsurfaceview);
        mSeekBar = findViewById(R.id.seek_bar);
        mXsPlayer = new XsPlayer();

        mXsPlayer.setXsGLSurfaceView(mXsGLSurfaceView);
        mSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                position = progress * mXsPlayer.getDuration() / 100;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                seek = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mXsPlayer.seek(position);
                seek = false;
            }
        });

        mXsPlayer.setOnPreparedListener(new OnPreparedListener() {
            @Override
            public void onPrepared() {
                Log.d(TAG, "准备好了，可以开始播放声音了");
                mXsPlayer.start();
            }
        });

        mXsPlayer.setOnLoadListener(new OnLoadListener() {
            @Override
            public void onLoad(boolean isLoad) {
                if (isLoad) {
                    Log.d(TAG, "加载中....");
                } else {
                    Log.d(TAG, "播放中....");
                }
            }
        });

        mXsPlayer.setOnPauseResumeListener(new OnPauseResumeListener() {
            @Override
            public void onPause(boolean isPause) {
                if (isPause) {
                    Log.d(TAG, "暂停中....");
                } else {
                    Log.d(TAG, "播放中....");
                }
            }
        });

        mXsPlayer.setOnTimeInfoListener(new OnTimeInfoListener() {
            @Override
            public void onTimeInfo(TimeInfoBean timeInfoBean) {
//                Log.d(TAG,timeInfoBean.toString());
                Message message = Message.obtain();
                message.what = 1;
                message.obj = timeInfoBean;
                mHandler.sendMessage(message);
            }
        });

        mXsPlayer.setOnErrorListener(new OnErrorListener() {
            @Override
            public void onError(int code, String msg) {
                Log.d(TAG, "code = " + code + " msg = " + msg);
            }
        });

        mXsPlayer.setOnCompleteListener(new OnCompleteListener() {
            @Override
            public void onComplete() {
                Log.d(TAG, "播放完成了");
            }
        });
    }

    public void begin(View view) {
//        mXsPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
//        mXsPlayer.setSource("http://ngcdn004.cnr.cn/live/dszs/index.m3u8");
        mXsPlayer.setSource("/storage/emulated/0/js.mp4");
        mXsPlayer.prepared();
    }

    public void pause(View view) {
        mXsPlayer.pause();
    }

    public void resume(View view) {
        mXsPlayer.resume();
    }

    public void stop(View view) {
        mXsPlayer.stop();
    }

    public void seek(View view) {
        mXsPlayer.seek(10);
    }

    public void next(View view) {
        mXsPlayer.playNext("/storage/emulated/0/123.mp4");
    }

    Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (msg.what == 1) {
                TimeInfoBean timeInfoBean = (TimeInfoBean) msg.obj;
                mTextView.setText(TimeUtils.getStringTime(timeInfoBean.getCurrentTime())
                        + "/" + TimeUtils.getStringTime(timeInfoBean.getTotalTime()));

                if (!seek && timeInfoBean.getTotalTime() > 0) {
                    mSeekBar.setProgress(timeInfoBean.getCurrentTime() * 100 / timeInfoBean.getTotalTime());
                }
            }
        }
    };

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        PermissionUtils.onRequestPermissionsResult(this, requestCode, permissions, grantResults);
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }
}
