#include"utils.h"
//pthread functions
#ifdef PTHREAD
bool StartThread(pthread_t* thread_id, void* thread_func(void*), void* param){
	int ret = pthread_create(thread_id, NULL, thread_func, param);
	if (ret != 0){
		perror("create thread!\n");
		return false;
	}
	return true;
}

void StopThread(pthread_t thread_id){
	pthread_cancel(thread_id);
}
void JoinThread(pthread_t& thread_id){
	pthread_join(thread_id, NULL);
}
#else
bool StartThread(std::thread* thread, void* thread_func(void*), void* param){
	*thread = std::thread(thread_func, param);
	if (!thread){
		perror("create thread!\n");
		return false;
	}
	return true;
}
void JoinThread(std::thread& t){
	t.join();
}
#endif


//time function
void GetTime(char* buffer){
	time_t now;
	struct tm* timenow;
	time(&now);
	timenow = localtime(&now);
	strcpy(buffer, asctime(timenow));
}

void BuildSockAddr(const char* ip, const uint16_t port, struct sockaddr_in* sock_addr){
	memset((char*)sock_addr, 0, sizeof(struct sockaddr_in));
	sock_addr->sin_family = AF_INET;
	sock_addr->sin_port = htons(port);
	sock_addr->sin_addr.s_addr = inet_addr(ip);
}

uint32_t Hex2Dec(char c){
    if(c >= '0' && c <= '9')
      return c - '0';
    else if(c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    else if(c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    printf("invalid hex char %c !\n", c);
    return 0;
}
