#include "VideoDecoder.h"

VideoDecoder::VideoDecoder(){
    freq = 300;   // frequency: 300 MHz
    fps  = 60;
    latency = freq / fps * 1e+6;
    startCycle = 0;
}

/**
 * @description: Load necessary data(ibp, mvs).
 */
void VideoDecoder::LoadData(){
    // Load frame types, include i/b/p frame.
    string bpath = "./idx/b/"+videoName, ppath = "./idx/p/"+videoName;
    ifstream bfile(bpath.c_str(), ios::in);
    string bline, pline, mvline, tmpStr;
    while(getline(bfile, bline)){
        typeList[atoi(bline.c_str())-1] = _bFrame;
    }
    bfile.close();
    ifstream pfile(ppath.c_str(), ios::in);
    while(getline(pfile, pline)){
        typeList[atoi(pline.c_str())-1] = _ipFrame;
    }
    pfile.close();

    // Load motion vectors
    string mvpath = "./mvs/"+videoName+".csv";
    ifstream mvfile(mvpath.c_str(), ios::in);
    vector<int> elems;
      
    while(getline(mvfile, mvline)){
        stringstream ss(mvline);
        while(getline(ss, tmpStr, ',')){
            elems.push_back(atoi(tmpStr.c_str()));
        }
        if (elems[6] < 0 || elems[7] < 0){
            // TODO: solve padding problem. 
            elems.clear();
            continue;
        }
        for (int i = elems[2] - 8; i >= 0; i -= 8) {
            for (int j = elems[3] - 8; j >= 0; j -= 8) {
                MvItem item(elems[0], elems[1], elems[4]+i, elems[5]+j, elems[6]+i, elems[7]+j);
                mvTable.push_back(item);
            }
        }
        elems.clear();
    }
    mvfile.close();
}

/**
 * @description: Generate decode order.
 */
void VideoDecoder::GenerateDecodeOrder(){
    Graph graph;
    // initial arc
    for(int i = 0; i < MAX_FRAMES_NUM; ++i){
        graph.visit[i] = false;
        for(int j = 0; j < MAX_FRAMES_NUM; ++j){
            graph.arc[i][j] = false;
        }
    }
    for(vector<MvItem>::iterator it = mvTable.begin(); it != mvTable.end(); ++it){
        graph.arc[it->bidx][it->refidx] = true;
    }
    bool flag = false;
    int framesNum = typeList.size(), ord = 0;
    for(int i = 0; i < framesNum; ++i){
        if(!graph.visit[i]){
            flag = false;
            for(int j = 0; j < framesNum; ++j){
                if(graph.arc[i][j] == true){
                    flag = true;
                    break;
                }
            }
            if(!flag){
                decodeOrd[ord] = i;   // order
                orderQueue.push(i);
                for(int h = 0; h < framesNum; ++h){
                    graph.arc[h][i] = false;
                }
                graph.visit[i] = true;
                ord += 1;
                i = 0;
            }
        }
    }
    map<int, int> reOrderList; // map<frameIdx, order>
    for(int i = 0; i < framesNum; i++){
        reOrderList[decodeOrd[i]] = i;
    }
    for(vector<MvItem>::iterator it = mvTable.begin(); it != mvTable.end(); ++it){
        it->dOrder = reOrderList[it->bidx];
    }
}

/**
 * @description: Sort mvTable according to decode order.
 */
void VideoDecoder::SortMvTable(){
    for(vector<MvItem>::iterator it = mvTable.begin(); it != mvTable.end();){
        if(typeList[it->bidx] == _ipFrame){
            swap(*it, mvTable.back());
            mvTable.pop_back();
        }
        else ++it;
    }
    sort(mvTable.begin(), mvTable.end());
}

// judge if a frame is bidirectional prediction frame
void VideoDecoder::JudgeBidPred(){
    for(int i = 0; i < mvTable.size()-1; ++i){
        if(mvTable[i].bidx == mvTable[i+1].bidx && mvTable[i].srcx == mvTable[i+1].srcx && mvTable[i].srcy == mvTable[i+1].srcy){
            mvTable[i].isBidPred = true, mvTable[i+1].isBidPred = true;
        }
    }
    map<int, int> bpfInBframe;  // frame_B_idx, bidPredFrame_nums
    for(vector<MvItem>::iterator it = mvTable.begin(); it != mvTable.end(); ++it){
        itemsInFrame[it->bidx]++;
        // if(it->isBidPred){
        //     bpfInBframe[it->bidx]++;
        // }
    }
    // for(map<int, int>::iterator it = itemsInFrame.begin(); it != itemsInFrame.end(); it++){
    //     it->second -= (bpfInBframe[it->first]/2);
    // }
}

/**
 * @description: HEVC Decoder preprocessing function.(external call interface)
 */
void VideoDecoder::Init(string vname){
    videoName = vname;
    LoadData();
    GenerateDecodeOrder();
    SortMvTable();
    JudgeBidPred();
}

/**
 * @description: Get decode order
 */
queue<int> VideoDecoder::GetDecodeOrder(){
    int frameNum = typeList.size();
    queue<int> order;
    for(int i = 0; i < frameNum; ++i){
        order.push(decodeOrd[i]);
    }
    return order;
}

map<int, FrameType> VideoDecoder::GetFramesType(){
    return typeList;
}

/**
 * @description: Get motion vectors of B-frame
 */
vector<MvItem> VideoDecoder::GetBframeMvs(){
    return mvTable;
}

void VideoDecoder::PrintTypeList(){
    for(int i = 0; i < typeList.size(); ++i){
        FrameType ftype = typeList[decodeOrd[i]];
        if(ftype == _ipFrame){
            cout<<"P ";
        }
        else cout<<"B ";
    }
    cout<<endl;
}

void VideoDecoder::VideoDecoderSim(){
    ++globalTimer;
    if(!orderQueue.empty()){
        if(timer == 0){
            startCycle = globalTimer;
        }
        ++timer;
        if(timer >= latency){
            int frameId = orderQueue.front(); 
            cout<<"Frame "<<frameId<<" completes video decoding, ("<<startCycle<<" -> "<<globalTimer<<")"<<endl;
            timer = 0;
            orderQueue.pop();
            // write decoded frames back to dram
            if(typeList[frameId] == _ipFrame){
                for(int row = 0, BASEROW = 56*frameId; row < 56; ++row){
                    for(int col = 0; col < 127; col+=8){
                        for(int bank = 0; bank < 8; ++bank){
                            bitset<3> b(bank);
                            bitset<15> r(row+BASEROW);
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
}