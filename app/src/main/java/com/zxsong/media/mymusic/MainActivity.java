package com.zxsong.media.mymusic;

import android.Manifest;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

import com.zxsong.media.mymusic.utils.PermissionUtils;
import com.zxsong.media.myplayer.listener.OnLoadListener;
import com.zxsong.media.myplayer.listener.OnPreparedListener;
import com.zxsong.media.myplayer.player.XsPlayer;

public class MainActivity extends AppCompatActivity {

    private XsPlayer mXsPlayer;

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
        mXsPlayer = new XsPlayer();
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

    }

    public void begin(View view) {
        mXsPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        mXsPlayer.prepared();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        PermissionUtils.onRequestPermissionsResult(this, requestCode, permissions, grantResults);
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }
}
