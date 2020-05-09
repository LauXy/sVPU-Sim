/*
 * @Author: Liu Xueyuan
 * @Date: 2020-03-04 21:12:28
 * @LastEditTime: 2020-05-09 22:09:15
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \RVPUSim\MvBuffer.cpp
 */
#include "MvBuffer.h"

void MvBuffer::Init(vector<MvItem> allMvs){
    mvTable = allMvs;
    reverse(mvTable.begin(), mvTable.end());
}

int MvBuffer::GetDramPage(MvItem item){
    return 56 * item.refidx + (item.dstx>>7<<3) + (item.dsty>>6);
}

void MvBuffer::GenerateLoadInstr(){
    string addrStr;
    uint64_t addr;
    for(int row = 0; row < 20; ++row){
        for(int col = 0; col < 127; col += 8){
            for(int bank = 0; bank < 8; ++bank){
                bitset<3> b(bank);
                bitset<15> r(row+DramAddr::mvItemBasePage);
                bitset<7> c(col);
                addrStr = '0'+ b.to_string() + r.to_string() + c.to_string() + "000000";
                bitset<32> a(addrStr);
                instrQ.push(make_pair(a.to_ullong(), DRAMSim::DATA_READ));
            }
        }
    }
}

MvReqGroup MvBuffer::GenerateAlignedReq(MvItem item){
    int idx = item.refidx, x = item.dstx, y = item.dsty;
    vector<int> Xs, Ys;
    if(x % 8){
        Xs.push_back((x>>3)<<3);
        Xs.push_back(((x>>3)+1)<<3);
    }
    else Xs.push_back(x);
    if(y % 8){
        Ys.push_back((y>>3)<<3);
        Ys.push_back(((y>>3)+1)<<3);
    }
    else Ys.push_back(y);
    MvReqGroup rg(item);
    for(int i = 0; i < Xs.size(); ++i){
        for(int j = 0; j < Ys.size(); ++j){
            int tmpx = Xs[i], tmpy = Ys[j];
            bitset<3> bank(tmpx % 128 / 32 * 2 + tmpy % 64 / 32);
            bitset<15> row(item.dramPage);
            bitset<7> col(tmpx % 128 % 32 / 8 * 32 + tmpy % 64 % 32 / 8 * 8);
            string addr = '0'+ bank.to_string() + row.to_string() + col.to_string() + "000000";
            rg.alignedReq.push_back(addr);
        }
    }
    return rg;
}

void MvBuffer::FindNext(){
    for(;prefetchPtr != content.end(); ++prefetchPtr){
        if(prefetchPtr->dramPage == -1){
            prefetchPtr->dramPage = GetDramPage(*prefetchPtr);
        }
        if(curPage == prefetchPtr->dramPage){
            break;
        }
    }
}

void MvBuffer::ResetPtr(){
    prefetchPtr = content.begin();
    curPage = GetDramPage(content.front());
}

int MvBuffer::GetCurPage(){
    return curPage;
}

void MvBuffer::MvBufferSim(){
    // if MV Buffer is empty, load mv items
    if(content.empty()){
        list<MvItem> tmp;
        for(int i = 0; i < size && !mvTable.empty(); ++i){
            tmp.push_back(mvTable.back());
            mvTable.pop_back();
        }
        Load(tmp);
        GenerateLoadInstr();
        ResetPtr();
        isMvBufferOk = false;
    }
    // when all mvitems returned to mvbuffer or mvbuffer is not empty
    if(isMvBufferOk){
        FindNext();
        if(prefetchPtr == content.end()){
            ResetPtr();        
        }
        if(isSegResOk[prefetchPtr->refidx]){
            inCacheQueue.push(GenerateAlignedReq(*prefetchPtr));
            prefetchPtr = content.erase(prefetchPtr);
        }
    }
}