/*
 * @Author: Liu Xueyuan
 * @Date: 2020-05-07 11:15:48
 * @LastEditTime: 2020-05-09 22:11:00
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\Cache.cpp
 */
#include "Cache.h"

void Way::AddData(pair<CacheAddr, CacheAddr> addr){
    CacheAddr xAddr = addr.first, yAddr = addr.second;
    bool isTagXExist = false;
    for(list<xNode>::iterator it = content.begin(); it != content.end(); ++it){
        if(it->xtag == xAddr.tag){
            isTagXExist = true;
            xNode tmpNode = *it;
            content.erase(it);
            it = content.insert(content.begin(), tmpNode);
            it->ylist.push_front(yAddr.tag);
            if(it->ylist.size() > yMaxLen){
                it->ylist.pop_back();
            }
            break;
        }
    }
    if(!isTagXExist){
        xNode newNode;
        newNode.xtag = xAddr.tag;
        newNode.ylist.push_back(yAddr.tag);
        content.push_front(newNode);
    }
}

bool Way::FindData(pair<CacheAddr, CacheAddr> addr){
    CacheAddr xAddr = addr.first, yAddr = addr.second;
    bool isFind = false;
    for(list<xNode>::iterator it = content.begin(); it != content.end(); ++it){
        if(it->xtag == xAddr.tag){
            for(list<string>::iterator itt = it->ylist.begin(); itt != it->ylist.end(); ++itt){
                if(*itt == yAddr.tag){
                    isFind = true;
                    string tmpStr = *itt;
                    it->ylist.erase(itt);
                    it->ylist.push_front(tmpStr);
                    break;
                }
            }
            xNode tmpNode = *it;
            content.erase(it);
            content.push_front(tmpNode);
            break;
        }
    }
    return isFind;
}

pair<CacheAddr, CacheAddr> Cache::GenerateCacheAddr(string dramAddr){
    bitset<4> xIndex(dramAddr.substr(15,4)), yIndex(dramAddr.substr(11,4).c_str());
    bitset<1> xBank(dramAddr[22]), yBank(dramAddr[20]);
    CacheAddr xAddr(dramAddr.substr(1,3), xIndex.to_ulong(), xBank.to_ulong());
    CacheAddr yAddr(dramAddr.substr(4,7)+dramAddr.substr(19,3), yIndex.to_ulong(), yBank.to_ulong());
    return make_pair(xAddr, yAddr);
}

bool Cache::CheckHit(string dramAddr){
    pair<CacheAddr, CacheAddr> addrPair = GenerateCacheAddr(dramAddr);
    CacheAddr xAddr = addrPair.first, yAddr = addrPair.second;
    bool isHit = cache[xAddr.bank][yAddr.bank].bank[xAddr.index][yAddr.index].FindData(addrPair);
    return isHit;
}

void Cache::Replace(string dramAddr){
    pair<CacheAddr, CacheAddr> addrPair = GenerateCacheAddr(dramAddr);
    CacheAddr xAddr = addrPair.first, yAddr = addrPair.second;
    cache[xAddr.bank][yAddr.bank].bank[xAddr.index][yAddr.index].AddData(addrPair);
}

bool Cache::CanMergeAccess(string dramAddr){
    for(list<MvReqGroup>::iterator it = pendingList.begin(); it != pendingList.end(); ++it){
        for(list<string>::iterator itt = it->alignedReq.begin(); itt != it->alignedReq.end(); ++itt){
            if(*itt == dramAddr){
                return true;
            }
        }
    }
    return false;
}

// convert bin to hex
string bin2hex(const string& bin) {
    string res = "0x";
    for (int i = 0; i < bin.size(); i += 4) {
        string tmp = bin.substr(i, 4);
        if (tmp[0] == '0') {
            if (tmp == "0000")res += '0';
            else if (tmp == "0001") res += '1';
            else if (tmp == "0010") res += '2';
            else if (tmp == "0011") res += '3';
            else if (tmp == "0100") res += '4';
            else if (tmp == "0101") res += '5';
            else if (tmp == "0110") res += '6';
            else res += '7';
        }
        else {
            if (tmp == "1000") res += '8';
            else if (tmp == "1001") res += '9';
            else if (tmp == "1010") res += 'a';
            else if (tmp == "1011") res += 'b';
            else if (tmp == "1100") res += 'c';
            else if (tmp == "1101") res += 'd';
            else if (tmp == "1110") res += 'e';
            else res += 'f';
        }
    }
    return res;
}

void Cache::CacheSim(){
    if(!inCacheQueue.empty()){
        MvReqGroup rg = inCacheQueue.front();
        inCacheQueue.pop();
        for(list<string>::iterator it = rg.alignedReq.begin(); it != rg.alignedReq.end();){
            bool isHit = CheckHit(*it);
            if(isHit){
                ++hitCnt;
                it = rg.alignedReq.erase(it);
            }
            else{
                if(!CanMergeAccess(*it)){
                    bitset<32> a(*it);
                    instrQ.push(make_pair(a.to_ullong(), DRAMSim::DATA_READ));
                }
                ++it;
            }
        }
        if(rg.alignedReq.empty()){
            outCacheQueue.push(rg.oriMvItem);
        }
        else{
            pendingList.push_back(rg);
        }
    }
    if(!returnCacheQueue.empty()){
        string callbackAddr = returnCacheQueue.front();
        returnCacheQueue.pop();
        Replace(callbackAddr);
        for(list<MvReqGroup>::iterator it = pendingList.begin(); it != pendingList.end();){
            for(list<string>::iterator itt = it->alignedReq.begin(); itt != it->alignedReq.end(); ++itt){
                if(*itt == callbackAddr){
                    itt = it->alignedReq.erase(itt);
                    break;
                }
            }
            if(it->alignedReq.empty()){
                outCacheQueue.push(it->oriMvItem);
                it = pendingList.erase(it);
            }
            else ++it;
        }
    }
}