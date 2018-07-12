package com.zxsong.media.mymusic;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import com.zxsong.media.myplayer.Demo;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Demo demo = new Demo();
        demo.testFfmpeg();
    }

}
