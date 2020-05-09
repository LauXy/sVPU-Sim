/*
 * @Author: your name
 * @Date: 2020-03-08 21:03:34
 * @LastEditTime: 2020-05-09 14:28:05
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \RVPUSim\GlobalVar.cpp
 */
#include "GlobalVar.h"

const int frameWidth = 854, frameHeight = 480;
bool isLayerOk = true, isNetOk = false;
int npuReadCnt = 0, npuWriteCnt = 0;

bool isMvBufferOk = false;
map<int, bool> isSegResOk;
map<int, bool> isDecodeOk;
map<int, bool> isMapResOk;
map<int, int> itemsInFrame;
queue<MvReqGroup> inCacheQueue;
queue<MvItem> outCacheQueue;
queue<string> returnCacheQueue;
queue<MvItem> outShifterQueue;