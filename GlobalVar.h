/*
 * @Author: Liu Xueyuan
 * @Date: 2020-03-08 20:58:09
 * @LastEditTime: 2020-05-11 09:18:35
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\GlobalVar.h
 */
#ifndef _GLOBALVAR_H_
#define _GLOBALVAR_H_
#include <map>
#include <vector>
#include <queue>
#include <list>
#include <iostream>
#include <fstream>
#include "MvItem.h"
#include "./DRAMSim/Transaction.h"

using namespace std;

extern const int frameWidth, frameHeight;
extern bool isLayerOk, isNetOk;
extern bool isMvBufferOk;          // whether MvBuffer can start execution, when mvitems all returned, the label is set true.
extern map<int, bool> isSegResOk;  // the frames finished nn and write back to dram
extern map<int, bool> isDecodeOk;  // only for ip frames, after decode and write callback, the label is set true.
extern map<int, bool> isMapResOk;  // only for b frames, after mapping and write callback, the label is set true
extern int npuReadCnt, npuWriteCnt;

enum FrameType{
    _ipFrame, _bFrame
};
enum DeviceType{
     _decoder, _mvbuffer, _cache, _mapbuffer, _npu
};
extern DeviceType operator++(DeviceType& dt);
extern map<int, int> itemsInFrame;
extern queue<MvReqGroup> inCacheQueue;  // MvBuffer -> Cache
extern queue<string> returnCacheQueue;  // dram -> Cache
extern queue<MvItem> outCacheQueue;     // Cache -> ShifterArray
extern queue<MvItem> outShifterQueue;   // ShifterArray -> MappingBuffer
extern map<int, int> decoderWriteCnt;
extern ofstream slog;
extern bool WRITE_LOG;
extern bool OUTPUT_NN_LAYER;
extern bool OUTPUT_ACCESS_INFO;
extern void Print(string str);
extern uint64_t decoderAccCnt, mvBufferAccCnt;
extern uint64_t cacheAccCnt, mapBufferAccCnt, npuAccCnt;

#endif