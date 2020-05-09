/*
 * @Author: your name
 * @Date: 2020-05-07 07:56:33
 * @LastEditTime: 2020-05-09 23:13:19
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\Cache.h
 */
#ifndef _CACHE_H_
#define _CACHE_H_

#include <list>
#include <vector>
#include <string>
#include <bitset>
#include "GlobalVar.h"
using namespace std;

class CacheAddr{
public:
    string tag;
    int index;
    int bank;
    CacheAddr(string t, int i, int b): tag(t), index(i), bank(b){}
};

typedef class XtagNode{
public:
    string xtag;
    list<string> ylist;
}xNode;

class Way{
private:
    int xMaxLen, yMaxLen;
public:
    Way():xMaxLen(8), yMaxLen(8){}
    list<xNode> content;  // string: dram addr
    void AddData(pair<CacheAddr, CacheAddr> addr);
    bool FindData(pair<CacheAddr, CacheAddr> addr);
};

class Bank{
public:
    int xLen, yLen;
public:
    vector<vector<Way> > bank;
    Bank(){
        xLen = 16, yLen = 16;
        vector<Way> tmp(yLen);
        bank.assign(xLen, tmp);
    }
};

class Cache{
private:
    int memId;
    int xBankLen, yBankLen;
    list<MvReqGroup> pendingList;
    vector<vector<Bank> > cache;
    pair<CacheAddr, CacheAddr> GenerateCacheAddr(string dramAddr); // pair<xAddr, yAddr>
    bool CanMergeAccess(string dramAddr);
public:
    queue<pair<uint64_t, DRAMSim::TransactionType> > instrQ;
    uint64_t hitCnt;
    Cache():memId(5), xBankLen(2), yBankLen(2){
        vector<Bank> tmp(yBankLen);
        cache.assign(xBankLen, tmp);
        hitCnt = 0;
    }
    bool CheckHit(string dramAddr);
    void Replace(string dramAddr);
    void CacheSim();
};

#endif