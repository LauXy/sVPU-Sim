/*
 * @Author: Liu Xueyuan
 * @Date: 2020-05-06 08:21:13
 * @LastEditTime: 2020-05-07 08:39:43
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \sVPU-Sim\Buffer.h
 */
#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <list>
#include <vector>
#include <iostream>
using namespace std;

template<class T>
class Buffer{
private:
    int memId;
    bool WillAcceptData(int addSize);
public:
    long long size;
    list<T> content;
    Buffer(int s, int id);
    bool Insert(T elem, int pos=0);
    T Remove(int pos);
    bool Load(list<T> elems, int pos=0);
    vector<T> Store(int pos, int len);
    void ShowList();
};

template<class T>
Buffer<T>::Buffer(int s, int id){
    size = s, memId = id;
}

template<class T>
bool Buffer<T>::WillAcceptData(int addSize){
    return content.size() + addSize <= size;
}

template<class T>
bool Buffer<T>::Insert(T elem, int pos){
    bool isAccept = WillAcceptData(1);
    if(isAccept){
        typename list<T>::iterator it = content.begin(); 
        for(int i = 0; i < pos; ++i){
            ++it;
        }
        content.insert(it, elem);
    }
    return isAccept;
}

template<class T>
T Buffer<T>::Remove(int pos){
    typename list<T>::iterator it = content.begin();
    for(int i = 0; i < pos; ++i){
        ++it;
    }
    T elem = *it;
    it = content.erase(it);
    return elem;
}

template<class T>
bool Buffer<T>::Load(list<T> elems, int pos){
    bool isAccept = WillAcceptData(elems.size());
    if(isAccept){
        typename list<T>::iterator it = content.begin();
        for(int i = 0; i < pos; ++i){
            ++it;
        }
        content.splice(it, elems);
    }
    return isAccept;
}

template<class T>
vector<T> Buffer<T>::Store(int pos, int len){
    vector<T> storeList;
    int i = 0, j = 0;
    for(typename list<T>::iterator it = content.begin(); i < len; ++j){
        if(j >= pos){
            storeList.push_back(*it);
            it = content.erase(it);
            ++i;
        }
        else ++it;
       
    }
    return storeList;
}

template<class T>
void Buffer<T>::ShowList(){
    for(typename list<T>::iterator it = content.begin(); it != content.end(); ++it){
        cout<<*it<<" ";
    }
    cout<<endl;
}

#endif