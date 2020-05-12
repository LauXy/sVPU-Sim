/*
 * @Author: Liu Xueyuan
 * @Date: 2020-05-08 21:39:44
 * @LastEditTime: 2020-05-11 09:11:49
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\IOBuffer.cpp
 */
#include "IOBuffer.h"

IOBuffer::IOBuffer(){
    memId = 0; 
	globalTimer = 0;
    string systemIniFilename = "ini/system.ini";
    string deviceIniFilename = "ini/DDR3_micron_32M_8B_x8_sg15.ini";
    memorySystem = new MultiChannelMemorySystem(deviceIniFilename, systemIniFilename, "", "", 2048, NULL, NULL);
    memorySystem->setCPUClockSpeed(0); 	// set the frequency ratio to 1:1
    read_cb = new Callback<IOBuffer, void, unsigned, uint64_t, uint64_t>(this, &IOBuffer::ReadComplete);
    write_cb = new Callback<IOBuffer, void, unsigned, uint64_t, uint64_t>(this, &IOBuffer::WriteComplete);
	memorySystem->RegisterCallbacks(read_cb, write_cb, NULL);
    trans = NULL;

	mvBufferReadCnt = 0;
	mappingWriteCnt = 0;
	segResWriteCnt = 0;
}

void IOBuffer::AddPending(const Transaction *t, uint64_t cycle, DeviceType src){
    if (t->transactionType == DATA_READ)
	{
		pendingReadRequests[t->address].push_back(make_pair(cycle, src)); 
	}
	else if (t->transactionType == DATA_WRITE)
	{
		pendingWriteRequests[t->address].push_back(make_pair(cycle, src)); 
	}
	else
	{
		ERROR("This should never happen"); 
		exit(-1);
	}
}

void IOBuffer::ReadComplete(unsigned id, uint64_t address, uint64_t done_cycle){
    map<uint64_t, list<pair<uint64_t, DeviceType> > >::iterator it;
	it = pendingReadRequests.find(address); 
	if (it == pendingReadRequests.end())
	{
		ERROR("Cant find a pending read for this one"); 
		exit(-1);
	}
	else
	{
		if (it->second.size() == 0)
		{
			ERROR("Nothing here, either"); 
			exit(-1); 
		}
	}
    latency = done_cycle - it->second.front().first;

    if(it->second.front().second == _mvbuffer){
        ++mvBufferReadCnt;
		if(mvBufferReadCnt == 2560){
			isMvBufferOk = true;
			mvBufferReadCnt = 0;
			cout<<"Mv Items are Loaded at clock cycle: "<<globalTimer<<endl;
		}
    }
	else if(it->second.front().second == _cache){
		bitset<32> addr(address);
		returnCacheQueue.push(addr.to_string());
	}
	else if(it->second.front().second == _npu){
		--npuReadCnt;
		if(npuReadCnt == 0){
			isLayerOk = true;
		}
	}

    pendingReadRequests[address].pop_front();
}

void IOBuffer::WriteComplete(unsigned id, uint64_t address, uint64_t done_cycle){
    map<uint64_t, list<pair<uint64_t, DeviceType> > >::iterator it;
	it = pendingWriteRequests.find(address); 
	if (it == pendingWriteRequests.end())
	{
		ERROR("Cant find a pending read for this one"); 
		exit(-1);
	}
	else
	{
		if (it->second.size() == 0)
		{
			ERROR("Nothing here, either"); 
			exit(-1); 
		}
	}

	latency = done_cycle - it->second.front().first;

	if(it->second.front().second == _decoder){
		bitset<32> addr(address);
		bitset<15> row(addr.to_string().substr(4,15));
		int frameId = row.to_ulong() / 56;
		--decoderWriteCnt[frameId];
		if(decoderWriteCnt[frameId] == 0){
			isDecodeOk[frameId] = true;
			cout<<"Frame "<<frameId<<" is written back to DRAM after decoding at clock cycle: "<<globalTimer<<endl;
		}
	}
	else if(it->second.front().second == _mapbuffer){
		++mappingWriteCnt;
		if(mappingWriteCnt == 7168){
			bitset<32> addr(address);
			bitset<15> row(addr.to_string().substr(4,15));
			int frameId = row.to_ulong() / 56;
			isMapResOk[frameId] = true;
			cout<<"Frame "<<frameId<<" is written back to DRAM after mapping at clock cycle: "<<globalTimer<<endl;
			mappingWriteCnt = 0;
		}
	}
	else if(it->second.front().second == _npu){
		bitset<32> addr(address);
		bitset<15> row(addr.to_string().substr(4,15));
		if(row.to_ulong() < DramAddr::mvItemBasePage){
			++segResWriteCnt;
			if(segResWriteCnt == 7168){
				segResWriteCnt = 0;
				int frameId = row.to_ulong() / 56;
				isSegResOk[frameId] = true;
			}
		}
		else{
			--npuWriteCnt;
			if(npuWriteCnt == 0){
				isNetOk = true;
			}
		}
	}

	pendingWriteRequests[address].pop_front();
}

void IOBuffer::MoveInstrToIOBuffer(DeviceType src, queue<pair<uint64_t, DRAMSim::TransactionType> >& instrQ){
	while(!instrQ.empty()){
		pair<uint64_t, DRAMSim::TransactionType> instr = instrQ.front();
		instrQ.pop();
		Req r(instr.first, instr.second, src);
		pendingRequestsList.push(r);
	}
}

void IOBuffer::IOBufferSim(){
	++globalTimer;
	if(!pendingRequestsList.empty()){
		Req r = pendingRequestsList.front();
		pendingRequestsList.pop();
		bool isWrite = r.transType == DATA_WRITE ? true : false;
		trans = new Transaction(r.transType, r.addr, NULL);
		if((*memorySystem).addTransaction(isWrite, r.addr)){
			AddPending(trans, globalTimer, r.src);
			trans = NULL;
		}
	}
	(*memorySystem).update();
}