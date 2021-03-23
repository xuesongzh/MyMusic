package com.zxsong.media.mymusic.utils;

/**
 * Created by zxsong on 2018/10/14.
 */

public class TimeUtils {

    public static String getStringTime(int time) {
        long hours = time / (60 * 60);
        long minutes = (time % (60 * 60)) / (60);
        long seconds = time % (60);

        String sh = "00";
        if (hours > 0) {
            if (hours < 10) {
                sh = "0" + hours;
            } else {
                sh = hours + "";
            }
        }

        String sm = "00";
        if (minutes > 0) {
            if (minutes < 10) {
                sm = "0" + minutes;
            } else {
                sm = minutes + "";
            }
        }

        String ss = "00";
        if (seconds > 0) {
            if (seconds < 10) {
                ss = "0" + seconds;
            } else {
                ss = seconds + "";
            }
        }

        if(time >= 3600) {
            return sh + ":" + sm + ":" + ss;
        }
        return sm + ":" + ss;
    }
}
