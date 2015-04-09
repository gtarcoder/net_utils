#ifndef CELL_LIST_H_
#define CELL_LIST_H_
#include<stdint.h>
#include<queue>
#include"mutex.h"

#define CELL_SIZE 491 //cell size 

//#define LOCK_IDLE

#define LOCK_WORK //use mutex lock for multi-thread operation on queue<Cell*>

using namespace Mutex;


struct Cell{
    uint32_t cell_id;

    uint32_t unicast_ip;    //unicast ip, this is not necessary, 4 bytes len
    
    int data_len;           //data length of cell_body

    int body_start_pos;     //the cell body's start position

    char cell_data[CELL_SIZE];        //pointer to the buffer where store cell body

    Cell():unicast_ip(0), data_len(0), body_start_pos(0){
    };

    ~Cell(){
    }
};

//function pointer that use Cell* as a function parameter
//used in PopAndProcsCell function
typedef void(* ProcsCellFunc)(Cell*);  

class CellList{
public:
    CellList();
    ~CellList();

    //initialize cell list by allocating some Cells at beginning of program
    //to avoid frequently allocating Cell later
    void Init(int init_cell_num);

    //in receive thread, push received udp pack into the cell list, 
   void PushPacket(char* udp_packet, int len);

   //in process thread,  pop cell struct from std::queue, and process the cell
   //remember after use the Cell pointer this function returned, delete it
   //
   //or we can call PopAndProcsCell function to avoid deleting cell* manually
   Cell* PopWorkCell(); 

   //pop idle cell
   Cell* PopIdleCell();

   //push idle cell into idle_list
   void RecycleCell(Cell*);

   //pop and process cell, when call this function, give a function pointer pointes
   //to function that process the newly popped cell, after processed, the cell should be enqueued into idle_list
   void PopAndProcsCell(ProcsCellFunc fp);

//private:
   //use two queues to contain cells
   //In receive thread, when received a packet, load its data into a cell popped from idle_list_ preferentially,
   //if idle_cell_list is empty, construct a new cell and load packet data into it. After loaded packet data, the cell is pushed into work_list_
   //to wait to be processed by process thread.
   //
   //In process thread, take a cell out from work_list_, and process it, after processed, the cell is pushed into the idle_list_ to 
   //wait to be used by receive thread
   //
    std::queue<Cell*> work_list_;   //queue contains cells which has not been processed

    std::queue<Cell*> idle_list_;   //queue contains cells which has been processed and
                                         //is waiting to load new data and be enqueued in work_list_

#ifdef LOCK_WORK
   MutexLock mutex_lock_work_;
   Condition condition_;
#endif

#ifdef LOCK_IDLE
   MutexLock mutex_lock_idle_;
#endif
};
#endif
