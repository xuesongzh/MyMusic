//
// Created by zxsong on 2018/9/19.
//

#include "XsAudio.h"

XsAudio::XsAudio(XsPlaystatus *playstatus) {
    this->playstatus = playstatus;
    queue = new XsQueue(playstatus);
}

XsAudio::~XsAudio() {

}


