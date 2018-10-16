//
// Created by zxsong on 2018/9/24.
//

#ifndef MYMUSIC_XSPLAYSTATUS_H
#define MYMUSIC_XSPLAYSTATUS_H


#include <string.h>

class XsPlaystatus {

public:
    bool exit;
    bool load;
    bool seek;

public:
    XsPlaystatus();

    ~XsPlaystatus();

};


#endif //MYMUSIC_XSPLAYSTATUS_H
