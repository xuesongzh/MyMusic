package com.zxsong.media.mymusic;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

import com.zxsong.media.myplayer.listener.OnPreparedListener;
import com.zxsong.media.myplayer.player.XsPlayer;

public class MainActivity extends AppCompatActivity {

    private XsPlayer mXsPlayer;

    private static final String TAG = "MainActivity";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mXsPlayer = new XsPlayer();
        mXsPlayer.setOnPreparedListener(new OnPreparedListener() {
            @Override
            public void onPrepared() {
                Log.d(TAG, "准备好了，可以开始播放声音了");
                mXsPlayer.start();
            }
        });

    }

    public void begin(View view) {
        mXsPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        mXsPlayer.prepared();
    }
}
