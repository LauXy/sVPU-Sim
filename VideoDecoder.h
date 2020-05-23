/*
 * @Author: Liu Xueyuan
 * @Date: 2020-03-01 23:30:31
 * @LastEditTime: 2020-05-09 10:37:29
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\VideoDecoder.h
 */
#ifndef _VIDEODECODER_H_
#define _VIDEODECODER_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <queue>
#include <bitset>
#include "MvItem.h"
#include "GlobalVar.h"

using namespace std;

const int MAX_FRAMES_NUM = 105;


class Graph {
public:
    Graph(){}
    bool visit[MAX_FRAMES_NUM];
    bool arc[MAX_FRAMES_NUM][MAX_FRAMES_NUM];
};

class VideoDecoder{
private:
    int freq;
    int fps;
    uint64_t timer = 0, globalTimer = 0;
    uint64_t latency;
    uint64_t startCycle;
    string videoName;
    vector<MvItem> mvTable;
    int decodeOrd[MAX_FRAMES_NUM];
    queue<int> orderQueue;
    map<int, FrameType> typeList;
    void LoadData();
    void GenerateDecodeOrder();
    void SortMvTable();
    void JudgeBidPred();
public:
    queue<pair<uint64_t, DRAMSim::TransactionType> > instrQ;
    VideoDecoder();
    void Init(string vname);
    queue<int> GetDecodeOrder();
    map<int, FrameType> GetFramesType();
    vector<MvItem> GetBframeMvs();
    void PrintTypeList(); // for debug
    void VideoDecoderSim();
};

#endif