/*
 * @Author: Liu Xueyuan
 * @Date: 2020-05-07 23:50:30
 * @LastEditTime: 2020-05-09 15:04:21
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\ShifterArray.h
 */
#ifndef _SHIFTERARRAY_H_
#define _SHIFTERARRAY_H_

#include "GlobalVar.h"
#include <queue>
using namespace std;

class ShifterArray{
private:
    uint64_t timer;
    uint64_t latency;
    void SetLatency(MvItem item);
public:
    ShifterArray(){
        timer = 0, latency = 0;
    }
    void ShifterArraySim();
};

#endif