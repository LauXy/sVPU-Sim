/*
 * @Author: your name
 * @Date: 2020-05-08 00:21:24
 * @LastEditTime: 2020-05-09 08:39:23
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\MappingBuffer.h
 */
#ifndef _MAPPINGBUFFER_H_
#define _MAPPINGBUFFER_H_

#include <bitset>
#include "GlobalVar.h"
#include "Buffer.h"
using namespace std;

class MappingBuffer: public Buffer<MvItem>{
private:
    list<MvItem> bidprePendingList;  // pending bid pre frames
public:
    queue<pair<uint64_t, DRAMSim::TransactionType> > instrQ;
    MappingBuffer():Buffer(19080, 6){}
    void MappingBufferSim();
};

#endif
