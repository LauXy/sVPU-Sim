/*
 * @Author: Liu Xueyuan
 * @Date: 2020-05-08 20:43:19
 * @LastEditTime: 2020-05-11 07:52:33
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\IOBuffer.h
 */
#ifndef _IOBUFFER_H_
#define _IOBUFFER_H_

#include <queue>
#include <bitset>
#include <stdio.h>
#include "GlobalVar.h"
#include "DramAddr.h"
#include "./DRAMSim/SystemConfiguration.h"
#include "./DRAMSim/MemorySystem.h"
#include "./DRAMSim/MultiChannelMemorySystem.h"
#include "./DRAMSim/IniReader.h"
using namespace std;

class Req{
public:
    uint64_t addr;
    DRAMSim::TransactionType transType;
    DeviceType src;
    Req(uint64_t a, DRAMSim::TransactionType tt, DeviceType dt){
        addr = a, transType = tt, src = dt;
    }
};

class IOBuffer{
private:
    int memId;
    uint64_t globalTimer;
    uint64_t latency;
    map<uint64_t, list<pair<uint64_t, DeviceType> > > pendingReadRequests;
    map<uint64_t, list<pair<uint64_t, DeviceType> > > pendingWriteRequests;
    queue<Req> pendingRequestsList;

    MultiChannelMemorySystem *memorySystem;
    Callback_t *read_cb, *write_cb;
    Transaction *trans;

    int mvBufferReadCnt;
    int mappingWriteCnt;
    int segResWriteCnt;

    void AddPending(const Transaction *t, uint64_t cycle, DeviceType src);
public:
    IOBuffer();
    void ReadComplete(unsigned id, uint64_t address, uint64_t done_cycle);
    void WriteComplete(unsigned id, uint64_t address, uint64_t done_cycle);
    void MoveInstrToIOBuffer(DeviceType src, queue<pair<uint64_t, DRAMSim::TransactionType> >& instrQ);
    void IOBufferSim();
};


#endif