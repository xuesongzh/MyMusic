package com.zxsong.media.myplayer.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

public class XsGLSurfaceView extends GLSurfaceView {

    private XsRender mRender;

    public XsGLSurfaceView(Context context) {
        this(context, null);
    }

    public XsGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        mRender = new XsRender(context);
        setRenderer(mRender);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        mRender.setOnRenderListener(new XsRender.OnRenderListener() {
            @Override
            public void onRender() {
                requestRender();
            }
        });
    }

    public void setYUVData(int width, int height, byte[] y, byte[] u, byte[] v) {
        if (mRender != null) {
            mRender.setYUVRenderData(width, height, y, u, v);
            requestRender();
        }
    }

    public XsRender getRender() {
        return mRender;
    }
}
