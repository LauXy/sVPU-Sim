/*
 * @Author: Liu Xueyuan
 * @Date: 2020-05-08 21:39:53
 * @LastEditTime: 2020-05-09 23:16:19
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\SvpuController.cpp
 */
#include "SvpuController.h"

SvpuController::SvpuController(string dataset){
    sysTimer = 0;
    decoder.Init(dataset);
    mvbuffer.Init(decoder.GetBframeMvs());
    npu.Init(decoder.GetDecodeOrder(), decoder.GetFramesType());
    DeviceType dt = _decoder;
    for(int i = 0; i < 5; ++i){
        deviceStateList[dt] = false;
        dt = (enum DeviceType)(dt + 1);
    }
    dtPointer = _decoder;
}

void SvpuController::RequestScheduling(){
    if(dtPointer == _decoder){
        if(!decoder.instrQ.empty()){
            iobuffer.MoveInstrToIOBuffer(_decoder, decoder.instrQ);
        }
    }
    else if(dtPointer == _mvbuffer){
        if(!mvbuffer.instrQ.empty()){
            iobuffer.MoveInstrToIOBuffer(_mvbuffer, mvbuffer.instrQ);
        }
    }
    else if(dtPointer == _cache){
        if(!cache.instrQ.empty()){
            uint64_t topAddr = cache.instrQ.front().first;
            bitset<32> addr(topAddr);
            bitset<15> row(addr.to_string().substr(4,15));
            int pageId = row.to_ulong();
            if(pageId != mvbuffer.GetCurPage()){
                queue<pair<uint64_t, DRAMSim::TransactionType> > tmpQ;
                while(!cache.instrQ.empty()){
                    pair<uint64_t, DRAMSim::TransactionType> topInstr = cache.instrQ.front();
                    bitset<32> a(topInstr.first);
                    bitset<15> r(a.to_string().substr(4,15));
                    if(r.to_ulong() == pageId){
                        tmpQ.push(topInstr);
                        cache.instrQ.pop();
                    }
                    else break;
                }
                iobuffer.MoveInstrToIOBuffer(_cache, tmpQ);
            }
        }
    }
    else if(dtPointer == _mapbuffer){
        if(!mapbuffer.instrQ.empty()){
            cout<<"Move Instr from mapbuffer to iobuffer"<<endl;
            iobuffer.MoveInstrToIOBuffer(_mapbuffer, mapbuffer.instrQ);
        }
    }
    else if(dtPointer == _npu){
        if(!npu.instrQ.empty()){
            iobuffer.MoveInstrToIOBuffer(_npu, npu.instrQ);
        }
    }
    dtPointer = (enum DeviceType)((dtPointer + 1) % 5);
}

void SvpuController::SvpuSim(){
    while(!npu.IsNnOrderEmpty()){
        ++sysTimer;
        decoder.VideoDecoderSim();
        mvbuffer.MvBufferSim();
        cache.CacheSim();
        shifter.ShifterArraySim();
        mapbuffer.MappingBufferSim();
        npu.NpuSim();
        RequestScheduling();
        iobuffer.IOBufferSim();
    }
    cout<<"End of simulation! Time consuming: "<<sysTimer<<", Cache hits: "<<cache.hitCnt<<endl;
}


int main(int argc, char* argv[]){
    SvpuController svpu(argv[1]);
    svpu.SvpuSim();
    return 0;
}