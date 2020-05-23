/*
 * @Author: Liu Xueyuan
 * @Date: 2020-03-07 23:01:00
 * @LastEditTime: 2020-05-09 08:45:12
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\DramAddr.h
 */
#ifndef _DRAMADDR_H_
#define _DRAMADDR_H_

#include <stdint.h>
using namespace std;

class DramAddr{
public:
    static const uint64_t segMapBasePage;  // Seg & Map & Decode
    static const uint64_t mvItemBasePage;
    static const uint64_t weightBasePage;
    static const uint64_t activeBasePage;
};

#endif