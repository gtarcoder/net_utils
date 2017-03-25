#include "raw_socket.h"
RawSocket::RawSocket(int mode) {
	// TODO Auto-generated constructor stub
	m_eWorkMode = mode;
	m_nSockfd = -1;
}

RawSocket::~RawSocket() {
	// TODO Auto-generated destructor stub
	if (m_nSockfd > 0)
	{
		close(m_nSockfd);
	}
}

void RawSocket::InitSocket(const char* interfaceName)
{
	strcpy(m_strInetName, interfaceName);						//绑定发送或接收的本地 网卡名
	//raw send
	if (m_eWorkMode == SEND)
	{	
		/*
        //不设置以太网帧的目的mac地址，而是通过系统路由表发送。使用这种方式即可。
		memset(&m_AddrSock, 0, sizeof(struct sockaddr_in));
		m_AddrSock.sin_family = AF_INET;
		if ((m_nSockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0)
		{
			printf("error\n");
			return;
		}
		unsigned int tmp = 1;
		if (setsockopt(m_nSockfd, IPPROTO_IP, IP_HDRINCL, &tmp, sizeof(tmp)) < 0)
		{
			printf("set raw sock opt error\n");
			return;
		}
		return;
        */
        //通过设置以太网帧的目的mac地址，构造以太网帧，并绑定网卡发送，使用这种方式 
        
		memset(&m_AddrSll, 0, sizeof(struct sockaddr_ll));
		if ((m_nSockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0)
		{
			perror("build send raw socket error!\n");
			return;
		}
        struct ifreq if_idx, if_mac;
        /* Get the index of the interface to send on */
        memset(&if_idx, 0, sizeof(struct ifreq));
        strncpy(if_idx.ifr_name, interfaceName, IFNAMSIZ-1);
        if (ioctl(m_nSockfd, SIOCGIFINDEX, &if_idx) < 0){
            perror("SIOCGIFINDEX");
            return;
        }
        /* Get the MAC address of the interface to send on */
        memset(&if_mac, 0, sizeof(struct ifreq));
        strncpy(if_mac.ifr_name, interfaceName, IFNAMSIZ-1);
        if (ioctl(m_nSockfd, SIOCGIFHWADDR, &if_mac) < 0){
            perror("SIOCGIFHWADDR");
            return;
        }

        m_AddrSll.sll_ifindex = if_idx.ifr_ifindex;
        m_AddrSll.sll_halen = ETH_ALEN;

        GetLocalMac(interfaceName); 
	}
    else{
	//raw recv
        if ((m_nSockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0)
        {
            perror("build recv raw socket error!\n");
            return;
        }
        struct ifreq ifstruct;
        strcpy(ifstruct.ifr_name, m_strInetName);
        if (ioctl(m_nSockfd, SIOCGIFINDEX, &ifstruct) < 0)
        {
            perror("get inet index error!\n");
            return;
        }

        memset(&m_AddrSll, 0, sizeof(struct sockaddr_ll));
        m_AddrSll.sll_family = AF_PACKET;
        m_AddrSll.sll_ifindex = ifstruct.ifr_ifindex;
        m_AddrSll.sll_protocol = htons(ETH_P_IP);

        if(bind(m_nSockfd, (struct sockaddr *) &m_AddrSll, sizeof(m_AddrSll)) == -1 )
        {
           perror("bind socket error!\n");
           return;
        }
    }
}

int RawSocket::SetPromisc()
{
    struct ifreq ifr;
    strcpy(ifr.ifr_name, m_strInetName);
    if(ioctl(m_nSockfd, SIOCGIFFLAGS, &ifr) == -1)
    {
		  perror("iotcl get promisc flag error!(setpromisc)\n");
		  return -1;
    }
    ifr.ifr_flags |= IFF_PROMISC;
    if(ioctl(m_nSockfd, SIOCSIFFLAGS, &ifr) == -1)
    {
		  perror("iotcl set promisc flag error!(setpromisc)\n");
		  return -1;
    }
    return 0;
}
int RawSocket::UnsetPromisc()
{
    struct ifreq ifr;
    strcpy(ifr.ifr_name, m_strInetName);
    if(ioctl(m_nSockfd, SIOCGIFFLAGS, &ifr) == -1)
    {
		perror("iotcl get promisc flag error!(unset promisc)\n");
		return -1;
    }
    ifr.ifr_flags &= ~IFF_PROMISC;
    if(ioctl(m_nSockfd, SIOCSIFFLAGS, &ifr) == -1)
    {
		perror("iotcl unset promisc flag error!(unset promisc)\n");
		return -1;
    }
    return 0;
}

int RawSocket::RecvPack(char*buf, int len)
{
	int recvedBytes = recv(m_nSockfd, buf, len, 0);
	if (recvedBytes == -1)
		printf("socket recved error!\n");
	return recvedBytes;
}

//发送以太网帧,buf 中的内容为原来的以太网帧（直接在原以太网帧上修改；为了避免当buf 为ip包时，再次的拷贝）
int RawSocket::SendEthPack(char* buf, int len, char* dst_mac_str){
    if(dst_mac_str){
        SetDstMac(dst_mac_str);
    }
	//自己设置以太网帧，然后从绑定网口发送。使用这种方式
	char* pos = buf;
	struct ether_header* ethHead = (struct ether_header*)pos;
	memcpy(ethHead->ether_shost, m_strSrcMac, MAC_LEN);
	memcpy(ethHead->ether_dhost, m_strDstMac, MAC_LEN);
	ethHead->ether_type = htons(ETH_P_IP);

	int sendBytes = sendto(m_nSockfd, buf, len, 0, (struct sockaddr*)&m_AddrSll, sizeof(sockaddr_ll));
	if (sendBytes < 0)
	{
		printf("sock send error!\n");
		return -1;
	}
	return sendBytes;
}

//发送以太网帧,buf 中的内容为 ip包
int RawSocket::SendPack(char* buf, int len, char* dst_mac_str)
{
    if(dst_mac_str){
        SetDstMac(dst_mac_str);
    }
	//自己设置以太网帧，然后从绑定网口发送。使用这种方式
	char etherPack[5000] = {0};
	char* pos = etherPack;
	struct ether_header* ethHead = (struct ether_header*)pos;
	memcpy(ethHead->ether_shost, m_strSrcMac, MAC_LEN);
	memcpy(ethHead->ether_dhost, m_strDstMac, MAC_LEN);
	ethHead->ether_type = htons(ETH_P_IP);
	pos += ETH_HEAD_LEN;
	memcpy(pos, buf, len);  //将ip包的内容拷贝到以太网帧中
	pos += len;

	int sendBytes = sendto(m_nSockfd, etherPack, pos - etherPack, 0, (struct sockaddr*)&m_AddrSll, sizeof(sockaddr_ll));
	if (sendBytes < 0)
	{
		printf("sock send error!\n");
		return -1;
	}
	return sendBytes;

    /*
    //不设置以太网帧，直接发送，通过系统路由表自动设置以太网帧. 使用这种方式
	struct iphdr *ipHead = (iphdr*)buf;
	m_AddrSock.sin_addr.s_addr = ipHead->daddr;

	int sendBytes =  sendto(m_nSockfd, buf, len, 0, (struct sockaddr *)&m_AddrSock, sizeof(m_AddrSock));
	if (sendBytes < 0)
	{
		perror("send ip pack error!\n");
	}
	return sendBytes;
    */
}

void RawSocket::GetLocalMac(const char* interface)
{
	int sock_mac;
	struct ifreq ifr_mac;
	sock_mac = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_mac == -1)
	{
		perror("create socket falied...mac\n");
		return;
	}

	memset(&ifr_mac, 0, sizeof(ifr_mac));
	strncpy(ifr_mac.ifr_name, interface, sizeof(ifr_mac.ifr_name) - 1);

	if ((ioctl(sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0)
	{
		printf("mac ioctl error\n");
		close(sock_mac);
		return;
	}
	memcpy(m_strSrcMac, ifr_mac.ifr_hwaddr.sa_data, MAC_LEN);
	close(sock_mac);
}

void RawSocket::SetDstMac(const char* dst_mac_str){
    if (strlen(dst_mac_str) != 17){
        printf("invalid mac addr str %s, length != 17\n", dst_mac_str);
        return;
    } 
    for(int i = 0; i < 6; i ++){
        uint8_t tmp = (Hex2Dec(dst_mac_str[3*i]) << 8) + Hex2Dec(dst_mac_str[3*i + 1]); 
        m_strDstMac[i] = tmp;
    }
    memcpy(m_AddrSll.sll_addr, m_strDstMac, MAC_LEN);
}
