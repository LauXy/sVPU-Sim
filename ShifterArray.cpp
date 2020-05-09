/*
 * @Author: Liu Xueyuan
 * @Date: 2020-05-07 23:57:33
 * @LastEditTime: 2020-05-09 22:01:46
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\ShifterArray.cpp
 */
#include "ShifterArray.h"

void ShifterArray::SetLatency(MvItem item){
    if(item.dstx % 8 != 0){
        if(item.dsty % 8 != 0){
            latency = 4;
        }
        else latency = 3;
    }
    else{
        if(item.dsty % 8 != 0){
            latency = 3;
        }
        else latency = 2;
    }
}

void ShifterArray::ShifterArraySim(){
    if(!outCacheQueue.empty()){
        if(timer == 0){
            MvItem item = outCacheQueue.front();
            SetLatency(item);
            ++timer;
        }
        else{
            ++timer;
            if(timer >= latency){
                timer = 0;
                MvItem item = outCacheQueue.front();
                outCacheQueue.pop();
                outShifterQueue.push(item);
            }
        }
    }
}