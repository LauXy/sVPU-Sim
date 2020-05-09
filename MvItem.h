/*
 * @Author: your name
 * @Date: 2020-05-06 15:33:34
 * @LastEditTime: 2020-05-07 20:29:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\MvItem.h
 */
#ifndef _MVITEM_H_
#define _MVITEM_H_
#include <string>
#include <list>
using namespace std;

class MvItem{
public:
    int bidx, refidx;
    int srcx, srcy;
    int dstx, dsty;
    int width, height;
    bool isBidPred;
    int dOrder;
    int dramPage;
    MvItem(int _bidx, int _ref, int _srcx, int _srcy, int _dstx, int _dsty, int _w=8, int _h=8, bool _isBidPred=false){
        bidx = _bidx, refidx = _ref;
        srcx = _srcx, srcy = _srcy;
        dstx = _dstx, dsty = _dsty;
        width = _w, height = _h;
        isBidPred = _isBidPred;
        dOrder = 0, dramPage = -1;
    }
    bool operator < (const MvItem& obj) const{
        if(dOrder == obj.dOrder){
            if(srcy == obj.srcy){
                return srcx < obj.srcx;
            }
            return srcy < obj.srcy;
        }
        return dOrder < obj.dOrder;
    }
};

class MvReqGroup{
public:
    MvItem oriMvItem;
    list<string> alignedReq;
    MvReqGroup(MvItem item):oriMvItem(item){}
};

#endif