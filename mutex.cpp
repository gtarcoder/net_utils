#ifdef PTHREAD

#include"mutex.h"
namespace Mutex{
MutexLock::MutexLock(){
#ifdef PTHREAD
	pthread_mutex_init(&mutex_, NULL);
#endif
}

MutexLock::~MutexLock(){
#ifdef PTHREAD
	pthread_mutex_destroy(&mutex_);
#endif
}


void MutexLock::Lock(){
#ifdef PTHREAD
	pthread_mutex_lock(&mutex_);
#else
	mutex_.lock();
#endif
}

void MutexLock::UnLock(){
#ifdef PTHREAD
	pthread_mutex_unlock(&mutex_);
#else
	mutex_.unlock();
#endif
}

#ifdef PTHREAD
pthread_mutex_t* MutexLock::GetPthreadMutex(){
	return &mutex_;
}
#else
std::mutex* MutexLock::GetPthreadMutex(){
	return &mutex_;
}
#endif

//Condition
Condition::Condition(MutexLock& lock):
	mutex_lock_(lock)
{
#ifdef PTHREAD
	pthread_cond_init(&pcond_, NULL);
#endif
}

Condition::~Condition(){
#ifdef PTHREAD
	pthread_cond_destroy(&pcond_);
#endif
}

void Condition::Wait(){
#ifdef PTHREAD
	pthread_cond_wait(&pcond_, mutex_lock_.GetPthreadMutex());
#endif
}

int Condition::WaitTimeOut(int timeout){
#ifdef PTHREAD
	struct timespec timer;
	timer.tv_sec = time(0) + timeout;
	timer.tv_nsec = 0;
	return pthread_cond_timedwait(&pcond_, mutex_lock_.GetPthreadMutex(), &timer);
#else
	return 0;
#endif
}

void Condition::Notify(){
#ifdef PTHREAD
	pthread_cond_signal(&pcond_);
#else
	pcond_.notify_one();
#endif
}

void Condition::NotifyAll(){
#ifdef PTHREAD
	pthread_cond_broadcast(&pcond_);
#else
	pcond_.notify_all();
#endif
}

};


#endif