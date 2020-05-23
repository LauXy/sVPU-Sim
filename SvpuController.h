/*
 * @Author: Liu Xueyuan
 * @Date: 2020-05-08 21:33:58
 * @LastEditTime: 2020-05-09 08:32:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\SvpuController.h
 */
#ifndef _SVPUCONTROLLER_H_
#define _SVPUCONTROLLER_H_

#include <string>
#include <map>
#include <algorithm>
#include "MvBuffer.h"
#include "Cache.h"
#include "ShifterArray.h"
#include "MappingBuffer.h"
#include "VideoDecoder.h"
#include "Npu.h"
#include "IOBuffer.h"

using namespace std;

class SvpuController{
private:
    uint64_t sysTimer;

    VideoDecoder decoder;
    Npu npu;
    MvBuffer mvbuffer;
    Cache cache;
    ShifterArray shifter;
    MappingBuffer mapbuffer;
    IOBuffer iobuffer;

    map<DeviceType, bool> deviceStateList;
    DeviceType dtPointer;

    void RequestScheduling();

public:
    SvpuController(string dataset);
    void CmdParse(int argc, char* argv[]);
    void SvpuSim();
};

#endif