#ifndef UDP_SOCKET_H_
#define UDP_SOCKET_H_
#include "utils.h"
#include"comm_socket.h"
class UdpSocket:public CommonSocket{
public:
    UdpSocket();
    virtual ~UdpSocket();

    bool Create();

    int SendTo(char* buf, int len, const char* dst_ip, uint16_t dst_port); 
    int SendTo(char* buf, int len, const struct sockaddr_in& target_addr); 
    int RecvFrom(char* buf, int buf_len, uint32_t* from_ip, uint16_t* from_port);
    int WaitMsg(int time_out, int* elapsed_time);
};
#endif
