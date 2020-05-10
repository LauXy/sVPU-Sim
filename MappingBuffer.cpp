/*
 * @Author: Liu Xueyuan
 * @Date: 2020-05-08 08:45:19
 * @LastEditTime: 2020-05-10 09:08:14
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\MappingBuffer.cpp
 */
#include "MappingBuffer.h"

void MappingBuffer::MappingBufferSim(){
    if(!outShifterQueue.empty()){
        MvItem item = outShifterQueue.front();
        outShifterQueue.pop();
        
        --itemsInFrame[item.bidx];

        if(item.isBidPred){
            bool isExistInPending = false;
            for(list<MvItem>::iterator it = bidprePendingList.begin(); it != bidprePendingList.end(); ++it){
                if(it->srcx == item.srcx && it->srcy == item.srcy && it->bidx == item.bidx){
                    isExistInPending = true;
                    it = bidprePendingList.erase(it);
                    break;
                }
            }
            if(!isExistInPending){
                bidprePendingList.push_back(item);
            }
        }

        if(itemsInFrame[item.bidx] == 0){
            // generate access requests to write frames to dram(instrQ)
            cout<<"Frame "<<item.bidx<<" out of mapping buffer"<<endl;
            for(int row = 0, BASEROW = 56*item.bidx; row < 56; ++row){
                for(int col = 0; col < 127; col+=8){
                    for(int bank = 0; bank < 8; ++bank){
                        bitset<3> b(bank);
                        bitset<15> r(BASEROW + row);
                        bitset<7> c(col);
                        string addr = '0' + b.to_string() + r.to_string() + c.to_string() + "000000";
                        bitset<32> dramAddr(addr);
                        instrQ.push(make_pair(dramAddr.to_ullong(), DRAMSim::DATA_WRITE));
                    }
                }
            }
        }
    }
}