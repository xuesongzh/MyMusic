package com.zxsong.media.myplayer.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

public class XsGLSurfaceView extends GLSurfaceView {

    private XsRender render;

    public XsGLSurfaceView(Context context) {
        this(context, null);
    }

    public XsGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);
        render = new XsRender(context);
        setRenderer(render);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        render.setOnRenderListener(new XsRender.OnRenderListener() {
            @Override
            public void onRender() {
                requestRender();
            }
        });
    }

    public void setYUVData(int width, int height, byte[] y, byte[] u, byte[] v) {
        if (render != null) {
            render.setYUVRenderData(width, height, y, u, v);
            requestRender();
        }
    }

    public XsRender getRender() {
        return render;
    }
}
