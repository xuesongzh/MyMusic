package com.zxsong.media.myplayer.opengl;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import com.zxsong.media.myplayer.R;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class XsRender implements GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener {


    public static final int RENDER_YUV = 1;
    public static final int RENDER_MEDIA_CODEC = 2;

    private final Context context;

    private final float[] vertexData = {
            -1f, -1f,
            1f, -1f,
            -1f, 1f,
            1f, 1f
    };

    private final float[] textureData = {
            0f, 1f,
            1f, 1f,
            0f, 0f,
            1f, 0f
    };

    private final FloatBuffer mVertexBuffer;
    private final FloatBuffer mTextureBuffer;
    private int renderType = RENDER_YUV;

    //yuv
    private int mProgramYuv;
    private int mAvPositionYuv;
    private int mAfPositionYuv;

    private int mSamplerY;
    private int mSamplerU;
    private int mSamplerV;
    private int[] mTextureIdYuv;

    private int mWidthYuv;
    private int mHeightYuv;
    private ByteBuffer y;
    private ByteBuffer u;
    private ByteBuffer v;

    //MediaCodec
    private int mProgramMediaCodec;
    private int mAvPositionMediaCodec;
    private int mAfPositionMediaCodec;
    private int mSamplerOESMediaCodec;
    private int mTextureIdMediaCodec;
    private SurfaceTexture mSurfaceTexture;
    private Surface surface;

    private OnSurfaceCreateListener onSurfaceCreateListener;
    private OnRenderListener onRenderListener;

    public XsRender(Context context) {
        this.context = context;
        mVertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(vertexData);
        mVertexBuffer.position(0);

        mTextureBuffer = ByteBuffer.allocateDirect(textureData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(textureData);
        mTextureBuffer.position(0);
    }

    public void setRenderType(int renderType) {
        this.renderType = renderType;
    }

    public void setOnSurfaceCreateListener(OnSurfaceCreateListener onSurfaceCreateListener) {
        this.onSurfaceCreateListener = onSurfaceCreateListener;
    }

    public void setOnRenderListener(OnRenderListener onRenderListener) {
        this.onRenderListener = onRenderListener;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        initRenderYUV();
        initRenderMediaCodec();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        if (renderType == RENDER_YUV) {
            renderYUV();
        } else if (renderType == RENDER_MEDIA_CODEC) {
            renderMediaCodec();
        }
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        if (onRenderListener != null) {
            onRenderListener.onRender();
        }
    }

    private void initRenderYUV() {
        String vertexSource = XsShaderUtil.readRawTxt(context, R.raw.vertex_shader);
        String fragmentSource = XsShaderUtil.readRawTxt(context, R.raw.fragment_yuv);
        mProgramYuv = XsShaderUtil.createProgram(vertexSource, fragmentSource);

        mAvPositionYuv = GLES20.glGetAttribLocation(mProgramYuv, "av_Position");
        mAfPositionYuv = GLES20.glGetAttribLocation(mProgramYuv, "af_Position");

        mSamplerY = GLES20.glGetUniformLocation(mProgramYuv, "sampler_y");
        mSamplerU = GLES20.glGetUniformLocation(mProgramYuv, "sampler_u");
        mSamplerV = GLES20.glGetUniformLocation(mProgramYuv, "sampler_v");

        // 创建纹理
        mTextureIdYuv = new int[3];
        GLES20.glGenTextures(3, mTextureIdYuv, 0);

        for (int i = 0; i < 3; i++) {
            // 绑定纹理
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextureIdYuv[i]);
            // 设置环绕和过滤方式
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        }
    }

    public void setYUVRenderData(int width, int height, byte[] y, byte[] u, byte[] v) {
        this.mWidthYuv = width;
        this.mHeightYuv = height;
        this.y = ByteBuffer.wrap(y);
        this.u = ByteBuffer.wrap(u);
        this.v = ByteBuffer.wrap(v);
    }

    private void renderYUV() {
        if (mWidthYuv > 0 && mHeightYuv > 0 && y != null && u != null && v != null) {
            GLES20.glUseProgram(mProgramYuv);

            GLES20.glEnableVertexAttribArray(mAvPositionYuv);
            GLES20.glVertexAttribPointer(mAvPositionYuv, 2, GLES20.GL_FLOAT, false, 8, mVertexBuffer);

            GLES20.glEnableVertexAttribArray(mAfPositionYuv);
            GLES20.glVertexAttribPointer(mAfPositionYuv, 2, GLES20.GL_FLOAT, false, 8, mTextureBuffer);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextureIdYuv[0]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, mWidthYuv, mHeightYuv, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, y);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextureIdYuv[1]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, mWidthYuv / 2, mHeightYuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, u);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextureIdYuv[2]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, mWidthYuv / 2, mHeightYuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, v);

            GLES20.glUniform1i(mSamplerY, 0);
            GLES20.glUniform1i(mSamplerU, 1);
            GLES20.glUniform1i(mSamplerV, 2);

            y.clear();
            u.clear();
            v.clear();
            y = null;
            u = null;
            v = null;
        }
    }

    private void initRenderMediaCodec() {
        String vertexSource = XsShaderUtil.readRawTxt(context, R.raw.vertex_shader);
        String fragmentSource = XsShaderUtil.readRawTxt(context, R.raw.fragment_mediacodec);
        mProgramMediaCodec = XsShaderUtil.createProgram(vertexSource, fragmentSource);

        mAvPositionMediaCodec = GLES20.glGetAttribLocation(mProgramMediaCodec, "av_Position");
        mAfPositionMediaCodec = GLES20.glGetAttribLocation(mProgramMediaCodec, "af_Position");
        mSamplerOESMediaCodec = GLES20.glGetUniformLocation(mProgramMediaCodec, "sTexture");

        int[] textureIds = new int[1];
        GLES20.glGenTextures(1, textureIds, 0);
        mTextureIdMediaCodec = textureIds[0];

        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_REPEAT);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_REPEAT);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);

        mSurfaceTexture = new SurfaceTexture(mTextureIdMediaCodec);
        surface = new Surface(mSurfaceTexture);
        mSurfaceTexture.setOnFrameAvailableListener(this);

        if (onSurfaceCreateListener != null) {
            onSurfaceCreateListener.onSurfaceCreate(surface);
        }
    }

    private void renderMediaCodec() {
        mSurfaceTexture.updateTexImage();
        GLES20.glUseProgram(mProgramMediaCodec);

        GLES20.glEnableVertexAttribArray(mAvPositionMediaCodec);
        GLES20.glVertexAttribPointer(mAvPositionMediaCodec, 2, GLES20.GL_FLOAT, false, 8, mVertexBuffer);

        GLES20.glEnableVertexAttribArray(mAfPositionMediaCodec);
        GLES20.glVertexAttribPointer(mAfPositionMediaCodec, 2, GLES20.GL_FLOAT, false, 8, mTextureBuffer);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mTextureIdMediaCodec);
        GLES20.glUniform1i(mSamplerOESMediaCodec, 0);
    }


    public interface OnSurfaceCreateListener {
        void onSurfaceCreate(Surface surface);
    }

    public interface OnRenderListener {
        void onRender();
    }


}
