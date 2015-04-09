#ifndef CELL_FACTORY_H_
#define CELL_FACTORY_H_
#include<stdint.h>

#define MAX_CELL_SIZE 1000
#define MAX_DATA_LEN 1000

//when pack or unpack cell,  use this pointer of MsgPacket or VideoPacket point
//to a positon in buffer, we can get or set the packet propertities automatically  EVEN 
//when considering the byte-alignment 
struct MsgPacket{

    uint16_t frame_head; //2 Bytes len frame head, bits 15-13 indicates the message type, 
                         //bits 12-11 indicates the block head flag, 00 is a complete block,  01 head part of a msg packet, 10 body, 11 tail
                         //bits 10-0 indicates the length of the following packet data
    char packet_data[MAX_DATA_LEN];
};

struct VideoPacket{
    uint8_t stream_format;
    uint8_t cell_type;
    uint16_t first_pack_tail;
    uint16_t body_length;
    char packet_data[MAX_DATA_LEN];
};


struct Cell{
    uint32_t cell_id;

    uint32_t unicast_ip;    //unicast ip, this is not necessary, 4 bytes len
    
    int data_len;           //data length of cell_body

    int body_start_pos;     //the cell body's start position

    char cell_data[MAX_CELL_SIZE]; 
    //pointer to buffer where store cell head and body,  in cell_data, we put all information of cell, ie. cell_id, unicast_ip, cel_type, etc

    Cell():unicast_ip(0), data_len(0), body_start_pos(0){
    };

    ~Cell(){
    }
};

class CellList;

class CellFactory{
public:
    CellFactory();
    ~CellFactory();

    bool PackMsgCell(char* msg, int len, CellList* cell_list);
    bool UnpackMsgCell();
    bool PackVideoCell(char* video, int len, CellList* cell_list);
    bool UnPackVideoCell();
private:
    //cell queue, in cell queue, we can store any kinds of cell(ie. Message Cell, Video Cell, File Cell)
    //use multi-thread to pack raw-data into cells and push cell into cell_list, so the synchronous and mutex should be considered
    //but for the consideration of efficiency, ?????
    CellList*  cell_list_; 
};
#endif
