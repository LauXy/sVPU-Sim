/*
 * @Author: your name
 * @Date: 2020-03-01 20:36:31
 * @LastEditTime: 2020-05-10 09:30:18
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \RVPUSim\MvBuffer.h
 */
#ifndef _MVBUFFER_H_
#define _MVBUFFER_H_

#include <list>
#include <queue>
#include <string>
#include <vector>
#include <algorithm>
#include <bitset>
#include "Buffer.h"
#include "GlobalVar.h"
#include "DramAddr.h"

using namespace std;


class MvBuffer: public Buffer<MvItem>{
private:
    int curPage;
    vector<MvItem> mvTable;
    list<MvItem>::iterator procPtr, prefetchPtr;
    int GetDramPage(MvItem item);
    void GenerateLoadInstr();
    MvReqGroup GenerateAlignedReq(MvItem item);
    void FindNext();
    void ResetPtr();
public:
    queue<pair<uint64_t, DRAMSim::TransactionType> > instrQ;
    MvBuffer():Buffer<MvItem>(5000, 4){ //32914
        procPtr = content.begin();
        prefetchPtr = content.begin();
        curPage = 0;
    }
    void Init(vector<MvItem> allMvs);
    int GetCurPage();
    void MvBufferSim();
};

#endif