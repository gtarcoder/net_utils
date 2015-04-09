#ifndef FIFO_QUEUE_H_
#define FIFO_QUEUE_H_
#define INIT_BUF_SIZE 1000000
class FifoQueue{
public:
    FifoQueue(int buf_size = INIT_BUF_SIZE);
    ~FifoQueue();

    bool PushPacket(char* packet, int len);
    bool PopCell(char* cell, int* len);
    bool PushCell(char* cell, int len);
    bool PopPacket(char* packet, int * len);
private:
    

};
#endif
