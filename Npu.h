/*
 * @Author: your name
 * @Date: 2020-03-03 20:19:25
 * @LastEditTime: 2020-05-09 08:25:23
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \rVPU\neuralNetworkSim.h
 */
#ifndef _NPU_H_
#define _NPU_H_

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <bitset>
#include <queue>
#include <map>
#include "GlobalVar.h"
#include "DramAddr.h"

using namespace std;

class NnLayer{
public:
    int layerId;
    int dependLayer;
    int weightSize;
    int inputSize;
    int outputSize;
    uint64_t computeSize;
    NnLayer(int lid, int did, int w, int i, int o, uint64_t c):
    layerId(lid), dependLayer(did), weightSize(w), 
    inputSize(i), outputSize(o), computeSize(c){}
};

// Type of neural network, SNN_T-small NN, LNN_T-large NN
enum NetMode{
    SNN_T, LNN_T
};

enum NpuIOType{
    READ_INPUTS, READ_WEIGHTS, WRITE_OUTPUTS
};

class Npu{
private:
    int weightFifoSize;    // weight fifo size
    int macNum;            // number of mac units
    int unifiedBufferSize; // unified buffer size
    uint64_t layerTimer, globalTimer;
    uint64_t startCycle;
    int curlayer;
    NetMode netMode;
    bool isLoadWeight;     // whether need to load weight
    queue<int> nnOrder;    // order of doing neural-network
    map<int, FrameType> frameType;
    map<NetMode, vector<NnLayer> > netParameters;
    void GenerateReqAddr(int transNum, int baseRow, DRAMSim::TransactionType transType);  // save in instrQ
    int GetTransPerLayer(NpuIOType io_t);
    uint64_t GetExeCyclesPerLayer();
    void NpuIoManager();
public:
    queue<pair<uint64_t, DRAMSim::TransactionType> > instrQ;
    Npu();
    void Init(queue<int> decodeOrder, map<int, FrameType> typeList);
    bool IsNnOrderEmpty();
    void NpuSim();
};

#endif