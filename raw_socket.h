#ifndef RAWSOCKET_H_
#define RAWSOCKET_H_
#include"utils.h"
class RawSocket {
public:
	RawSocket(int mode = 0/*默认设置为从指定网卡接收*/);
	virtual ~RawSocket();
	void	InitSocket(const char* interfaceName);
	int 	SetPromisc();	//设置网卡为混杂模式
	int		UnsetPromisc(); //取消网卡混杂模式
	int		SendPack(char*buf, int len, char* dst_mac_str = NULL);     //指定网卡发送IP包（如果需要），参数buf 为ip包
	int		SendEthPack(char*buf, int len, char* dst_mac_str = NULL);     //指定网卡发送IP包（如果需要）, 参数buf 为以太网帧
	int		RecvPack(char*buf, int len);	 //从网卡接受IP包（实际为以太网帧）
	void	GetLocalMac(const char* interface);	 //获得网卡mac地址
    void    SetDstMac(const char* dst_mac);
private:
    enum{
        RECV = 0,
        SEND = 1
    };
    enum{
        MAC_LEN = 6,
        ETH_HEAD_LEN = 14
    };
	int   			        m_eWorkMode;	//rawsocket 工作模式（指定网卡接收 或 指定网卡发送)
	int 					m_nSockfd;
	char					m_strInetName[100]; //本地网卡名称 eth0， eth1

	unsigned char			m_strSrcMac[6]; //本地指定的网卡mac地址（用于发送时构造以太网帧）
	unsigned char			m_strDstMac[6];	//指定网卡发送时的对端mac地址
	struct sockaddr_in		m_AddrSock;
	struct sockaddr_ll 		m_AddrSll;
};
#endif /* RAWSOCKET_H_ */
