package com.zxsong.media.myplayer.util;

import android.media.MediaCodecList;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by zxsong on 2021/4/3.
 */

public class XsVideoSupportUtil {
    private static Map<String, String> codecMap = new HashMap<>();

    static {
        codecMap.put("h264", "video/avc");
    }

    // 解码器类型
    public static String findVideoCodecName(String ffcodename) {
        if (codecMap.containsKey(ffcodename)) {
            return codecMap.get(ffcodename);
        }
        return "";
    }

    public static boolean isSupportCodec(String ffcodecname) {
        boolean supportvideo = false;
        int count = MediaCodecList.getCodecCount();
        for (int i = 0; i < count; i++) {
            String[] types = MediaCodecList.getCodecInfoAt(i).getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equals(findVideoCodecName(ffcodecname))) {
                    supportvideo = true;
                    break;
                }
            }
            if (supportvideo) {
                break;
            }
        }
        return supportvideo;
    }
}
