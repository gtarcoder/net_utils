#include<assert.h>
#include<iostream>
#include<string.h>
#include<stdio.h>
#include"cell_list.h"
using namespace std;

#ifdef LOCK_WORK
CellList::CellList(int num):
condition_(mutex_lock_work_){
    Init(num);
}
#else
CellList::CellList(int num){
    Init(num);
}
#endif

CellList::~CellList(){
    while(! work_list_.empty()){
        delete work_list_.front();
        work_list_.pop();
    }

    while(! idle_list_.empty()){
        delete idle_list_.front();
        idle_list_.pop();
    }

}

void CellList::Init(int init_cell_num){
    Cell* cell;
    for(int i = 0; i < init_cell_num; i ++){
        cell = new Cell();
        if (! cell){
            printf("allocate cell failed\n");
            continue;
        }
        idle_list_.push(cell);
    }
}
//load packet data into cell struct and enqueue cell struct into work queue
void CellList::PushPacket(char* packet, int packet_len){
    assert(packet_len  <= CELL_SIZE);

    Cell* cell = PopIdleCell();

    if (! cell){ //need to allocate a new Cell
        if (! cell){
            cell = new Cell();
            printf("allocate cell failed...\n");
            return;
        }
    }
    
    memcpy(cell->cell_data, packet, packet_len);
    cell->data_len = packet_len;

#ifdef LOCK_WORK
    MutexLockGuard guard(mutex_lock_work_);
#endif

    work_list_.push(cell);

#ifdef LOCK_WORK
    if (work_list_.size() == 1){
        condition_.Notify();
    }
#endif
}


Cell* CellList::PopIdleCell(){
#ifdef LOCK_IDLE
    MutexLockGuard guard(mutex_lock_idle_);
#endif
    if (idle_list_.empty()){
        return NULL;
    }
    Cell* result = idle_list_.front();
    idle_list_.pop();
    return result;
}

Cell* CellList::PopWorkCell(){
#ifdef LOCK_WORK
    MutexLockGuard guard(mutex_lock_work_);
    while(work_list_.empty()){
        condition_.Wait();
    }
#else
    if (work_list_.empty()){
        usleep(200);    //sleep in case that cpu is overloaded
        return NULL;
    }
#endif

    Cell* result = work_list_.front();
    work_list_.pop();

    return result;
}

void CellList::PopAndProcsCell(ProcsCellFunc fp){

    Cell* cell = PopWorkCell();
    if (!cell){
        return;
    }

    //call the fp to process cell
    (*fp)(cell);

    //recycle cell by enqueuing it into idle_list
    RecycleCell(cell);

//    cout << "work queue's size = " << work_list_.size() << ", idle queue's size = " << idle_list_.size() << endl;
}


void CellList::RecycleCell(Cell* cell){
#ifdef LOCK_IDLE
    MutexLockGuard guard(mutex_lock_idle_);
#endif
    idle_list_.push(cell);
}
