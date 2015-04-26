#include<assert.h>
#include<iostream>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>
#include"db_cell_list.h"
using namespace std;

DbCellList::DbCellList():
condition_(mutex_lock_work_){
    write_list_ptr_ = &work_list1_;
    read_list_ptr_ = &work_list2_;
    read_list_empty_ = true;
}

DbCellList::~DbCellList(){
    while(! work_list1_.empty()){
        delete work_list1_.front();
        work_list1_.pop();
    }
    while(! work_list2_.empty()){
        delete work_list2_.front();
        work_list2_.pop();
    }
    while(! idle_list_.empty()){
        delete idle_list_.front();
        idle_list_.pop();
    }
}
void DbCellList::Init(int init_cell_num){
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
void DbCellList::PushPacket(char* packet, int packet_len){
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

    MutexLockGuard guard1(mutex_lock_work_);

#ifdef LOCK_EXCHGE
    MutexLockGuard guard2(mutex_lock_chgeptr_);
#endif

    write_list_ptr_->push(cell); 
    if (read_list_empty_ && write_list_ptr_->size() > MIN_READ_CELL_NUM){
        condition_.Notify();
    }
}

Cell* DbCellList::PopWorkCell(){
    if (read_list_ptr_->empty()){
		 MutexLockGuard lock_guard1(mutex_lock_work_);
         read_list_empty_ = true;
         while(read_list_empty_){
             if (condition_.WaitTimeOut(MAX_EXCHANGE_TIME_SPAN) == ETIMEDOUT){ //wait for data to timeout, exchange ptr forcely
                if (! write_list_ptr_->empty()){

#ifdef LOCK_EXCHGE
    				MutexLockGuard guard2(mutex_lock_chgeptr_);
#endif

                    ExchangeRwPtr();
                }                                                             
             }else{//else, we receive a notification from write thread that write buffer has enough cell to read(> MIN_READ_CELL_NUM)
                ExchangeRwPtr();
             }
         }
    }

    Cell* result = read_list_ptr_->front();
    read_list_ptr_->pop();
    return result;
}

void DbCellList::ExchangeRwPtr(){

    CellQueuePtr tmp = write_list_ptr_;
    write_list_ptr_ = read_list_ptr_;
    read_list_ptr_ = tmp;
    read_list_empty_ = false;
}

Cell* DbCellList::PopIdleCell(){
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
void DbCellList::PopAndProcsCell(ProcsCellFunc fp){
    Cell* cell = PopWorkCell(); //this operation will block here if no cell can be popped
    assert(cell != NULL);
    //call the fp to process cell
    (*fp)(cell);
    //recycle cell by enqueuing it into idle_list
    RecycleCell(cell);
}
void DbCellList::RecycleCell(Cell* cell){
#ifdef LOCK_IDLE
    MutexLockGuard guard(mutex_lock_idle_);
#endif
    idle_list_.push(cell);
}
