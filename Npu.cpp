#include "Npu.h"

Npu::Npu(){
    ifstream iniFile;
    iniFile.open("./ini/npu.ini");
    if(!iniFile.is_open()){
        cout<<"Cannot open npu.ini file!"<<endl;
    }
    string line;
    map<string, uint64_t> npuConfig;
    while(getline(iniFile, line)){
        if(line != "\r"){
            line = line.substr(0, line.find(';'));
            if(!line.empty()){
                int equpos = line.find('=');

                string key = line.substr(0, equpos);
                key.erase(0, key.find_first_not_of(" "));
                key.erase(key.find_last_not_of(" ") + 1);

                string val = line.substr(equpos + 1);
                val.erase(0, val.find_first_not_of(" "));
                val.erase(val.find_last_not_of(" ") + 1);

                istringstream iss(val);
                iss >> npuConfig[key];
            }
        }
    }
    iniFile.close();
    macNum = npuConfig["MAC_NUM"];
    weightFifoSize = npuConfig["WEIGHT_FIFO_SIZE"];
    unifiedBufferSize = npuConfig["UNIFIED_BUFFER_SIZE"];

    string lnnPath = "./ini/largeNnConfig.csv", snnPath = "./ini/smallNnConfig.csv";
    ifstream archFile(snnPath, ios::in);  
    string lineStr, str;
    while(getline(archFile, lineStr))  
    {  
        stringstream ss(lineStr);  
        vector<string> lineArray;  
        while (getline(ss, str, ','))  
            lineArray.push_back(str);
        uint64_t com;
        istringstream iss(lineArray[12]);
        iss >> com;
        NnLayer netLayer(atoi(lineArray[0].c_str()), atoi(lineArray[13].c_str()), atoi(lineArray[9].c_str()), atoi(lineArray[10].c_str()), atoi(lineArray[11].c_str()), com);
        netParameters[SNN_T].push_back(netLayer);
    }
    archFile.close();
    archFile.open(lnnPath, ios::in);
    while(getline(archFile, lineStr))  
    {  
        stringstream ss(lineStr);  
        vector<string> lineArray;  
        while (getline(ss, str, ','))  
            lineArray.push_back(str);
        uint64_t com;
        istringstream iss(lineArray[12]);
        iss >> com;
        NnLayer netLayer(atoi(lineArray[0].c_str()), atoi(lineArray[13].c_str()), atoi(lineArray[9].c_str()), atoi(lineArray[10].c_str()), atoi(lineArray[11].c_str()), com);
        netParameters[LNN_T].push_back(netLayer);
    }
    archFile.close();    
}

void Npu::Init(queue<int> decodeOrder, map<int, FrameType> typeList){
    isLoadWeight = true;
    curlayer = 0;
    layerTimer = 0, globalTimer = 0;
    startCycle = 0;
    nnOrder = decodeOrder;
    netMode = LNN_T;
    frameType = typeList;
}

bool Npu::IsNnOrderEmpty(){
    return nnOrder.empty();
}

int Npu::GetTransPerLayer(NpuIOType ioType){
    NnLayer thisLayer = netParameters[netMode][curlayer];
    int s_weight = thisLayer.weightSize;  // size of weights
    int s_input  = thisLayer.inputSize;   // size of inputs
    int s_output = thisLayer.outputSize;  // size of outputs
    int layersNum = netParameters[netMode].size();
    int transNum;
    
    if(ioType == READ_WEIGHTS){      
        // 64 Bytes/Transaction, load weights per layer from dram
        transNum = s_weight >> 6;
    }
    else if(ioType == READ_INPUTS){
        NnLayer preLayer = netParameters[netMode][curlayer-1];
        if(curlayer==0 || preLayer.layerId!=thisLayer.dependLayer){
            // if current layer is Layer-0 or the previous layer does not depend on this layer, then read all inputs from dram.
            transNum = s_input >> 6;
        }
        else{
            // the previous layer depends on this layer, read inputs from unified buffer and dram.
            if(s_input <= unifiedBufferSize){
                // inputs is tiny enough to be held in the on-chip buffer
                transNum = 0;
            }
            else{
                transNum = (s_input - unifiedBufferSize) >> 6;
            }
        }
    }
    else{
        // ioType == WRITE_OUTPUTS
        NnLayer postLayer = netParameters[netMode][curlayer+1];
        if(curlayer==layersNum-1 || postLayer.dependLayer!=thisLayer.layerId){
            // if current layer is the last layer or the next layer does not depend on current layer, then all outputs should be write back to dram.
            transNum = s_output >> 6;
        }
        else{
            // if the next layer depends on current layer, then outputs can be write to both on-chip buffer and dram.
            if(s_output <= unifiedBufferSize){
                // outpus is small enough that can be held in on-chip buffer
                transNum = 0;
            }
            else{
                transNum = (s_output - unifiedBufferSize) >> 6;
            }
        }
    }
    return transNum;
}

uint64_t Npu::GetExeCyclesPerLayer(){
    if(curlayer != -1){
        return netParameters[netMode][curlayer].computeSize / macNum;
    }
    return 0;
}

void Npu::GenerateReqAddr(int transNum, int baseRow, DRAMSim::TransactionType transType){
    int cnum = transNum % 16;
    int bnum = transNum / 16 % 8;
    int rnum = transNum / 128;
    for(int row = 0; row < rnum; ++row){
        for(int bank = 0; bank < 8; ++bank){
            for(int col = 0; col < 127; col+=8){
                bitset<3> b(bank);
                bitset<15> r(baseRow + row);
                bitset<7> c(col);
                string addrStr = '0'+ b.to_string() + r.to_string() + c.to_string() + "000000";
                bitset<32> a(addrStr);
                instrQ.push(make_pair(a.to_ullong(), transType));
            }
        }
    }
    for(int bank = 0; bank < bnum; ++bank){
        for(int col = 0; col < 127; col+=8){
            bitset<3> b(bank);
            bitset<15> r(baseRow + rnum);
            bitset<7> c(col);
            string addrStr = '0'+ b.to_string() + r.to_string() + c.to_string() + "000000";
            bitset<32> a(addrStr);
            instrQ.push(make_pair(a.to_ullong(), transType));
        }
    }
    for(int col = 0; col < cnum; ++col){
        bitset<3> b(bnum);
        bitset<15> r(baseRow + rnum);
        bitset<7> c(col*8);
        string addrStr = '0'+ b.to_string() + r.to_string() + c.to_string() + "000000";
        bitset<32> a(addrStr);
        instrQ.push(make_pair(a.to_ullong(), transType));
    }
}

void Npu::NpuIoManager(){
    int wTransNum = GetTransPerLayer(READ_WEIGHTS);
    GenerateReqAddr(wTransNum, DramAddr::weightBasePage, DRAMSim::DATA_READ);
    npuReadCnt += wTransNum;
    if(isLoadWeight){
        int iTransNum = GetTransPerLayer(READ_INPUTS);
        GenerateReqAddr(iTransNum, DramAddr::activeBasePage, DRAMSim::DATA_READ);
        npuReadCnt += iTransNum;
    }
    int oTransNum = GetTransPerLayer(WRITE_OUTPUTS);
    GenerateReqAddr(oTransNum, DramAddr::activeBasePage, DRAMSim::DATA_WRITE);
    npuWriteCnt += oTransNum;
}

void Npu::NpuSim(){
    ++globalTimer;
    if(!nnOrder.empty()){
        int curframe = nnOrder.front();
        if(frameType[curframe] == _ipFrame && isDecodeOk[curframe]){
            if(curlayer >= 103 && isNetOk){
                // large net finished
                cout<<"I/P Frame "<<curframe<<" completes the neural network.("<<startCycle<<" -> "<<globalTimer<<")"<<endl;
                // then write seg results to dram
                for(int row = 0, BASEROW = 56*curframe; row < 56; ++row){
                    for(int col = 0; col < 127; col+=8){
                        for(int bank = 0; bank < 8; ++bank){
                            bitset<3> b(bank);
                            bitset<15> r(row+BASEROW);
                            bitset<7> c(col);
                            string addr = '0'+ b.to_string() + r.to_string() + c.to_string() + "000000";
                            bitset<32> dramAddr(addr);
                            instrQ.push(make_pair(dramAddr.to_ullong(), DRAMSim::DATA_WRITE));
                        }
                    }
                }
                isLayerOk = true, isNetOk = false;
                curlayer = 0;
                layerTimer = 0;
                nnOrder.pop();
            }
            else{                
                if(layerTimer >= GetExeCyclesPerLayer() && isLayerOk && curlayer < 103){
                    // cout<<"ROI SegNet - Layer "<<curlayer<<" completed at clock cycle: "<<globalTimer<<endl;
                    if(curlayer == 0){
                        startCycle = globalTimer;
                    }
                    ++curlayer;
                    layerTimer = 0;
                    isLayerOk = false;
                    isNetOk = false;
                    NpuIoManager();
                }
                ++layerTimer;
            }
        }
        else{
            // curframe is a B-frame's mapping result
            if(isMapResOk[curframe]){
                if(curlayer >= 2 && isNetOk){
                    cout<<"B Frame "<<curframe<<" completes the neural network.("<<startCycle<<" -> "<<globalTimer<<")"<<endl;
                    // then write seg results to dram
                    for(int row = 0, BASEROW = 56*curframe; row < 56; ++row){
                        for(int col = 0; col < 127; col+=8){
                            for(int bank = 0; bank < 8; ++bank){
                                bitset<3> b(bank);
                                bitset<15> r(row+BASEROW);
                                bitset<7> c(col);
                                string addr = '0'+ b.to_string() + r.to_string() + c.to_string() + "000000";
                                bitset<32> dramAddr(addr);
                                instrQ.push(make_pair(dramAddr.to_ullong(), DRAMSim::DATA_WRITE));
                            }
                        }
                    }
                    isLayerOk = true, isNetOk = false;
                    curlayer = 0;
                    layerTimer = 0;
                    nnOrder.pop();
                    if(!nnOrder.empty() && frameType[nnOrder.front()] == _bFrame){
                        isLoadWeight = false;
                    }
                    else isLoadWeight = true;
                }
                else{
                    if(layerTimer >= GetExeCyclesPerLayer() && isLayerOk && curlayer < 2){
                        // cout<<"DenoiseNet - Layer "<<curlayer<<" completed at clock cycle: "<<globalTimer<<endl;
                        if(curlayer == 0){
                            startCycle = globalTimer;
                        }
                        ++curlayer;
                        layerTimer = 0;
                        startCycle = globalTimer;
                        isLayerOk = false, isNetOk = false;
                        NpuIoManager();
                    }
                    ++layerTimer;
                }
            }
        }
    }
}