#include<iostream>
#include"utils.h"
#include"db_cell_list.h"
#include"mutex.h"
using namespace std;
void* ReadFunc(void* param){
    DbCellList* cell_list = (DbCellList*)param;

    char time_str[300];

    while(true){

        Cell* cell = cell_list->PopWorkCell();

        if (!cell){
            continue;
        }

        int test_data = *(int*)cell->cell_data;

//        printf("in read thread, test_data = %d\n", test_data);
//        printf("work list size %d, idle list size %d\n",  cell_list->work_list_.size(), cell_list->idle_list_.size());
        if (test_data % 1000000 == 0){
            GetTime(time_str);
            printf("in read thread, test_data = %d\n", test_data);
            printf("time : %s\n\n\n", time_str);
        }
        cell_list->RecycleCell(cell);
    }
}
void* WriteFunc(void* param){
    DbCellList* cell_list = (DbCellList*)param;
    int test_data = 0;
    while(true){
        cell_list->PushPacket((char*)&test_data, sizeof(int));
        test_data ++;
        usleep(10);
    }
}

DbCellList cell_list;

int main(){
    cell_list.Init(100);
    pthread_t read_thread, write_thread;
    StartThread(&write_thread, WriteFunc, &cell_list);
    
    StartThread(&read_thread, ReadFunc, &cell_list);

    pthread_join(write_thread, NULL);
    pthread_join(read_thread, NULL);
    return 0;
}
