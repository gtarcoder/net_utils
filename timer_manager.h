/*************************************************************************
	> File Name: timer_manager.h
	> Author   : JasonSun 
	> Mail     : skc361@163.com 
	> Created Time: 2015年04月07日 星期二 13时54分34秒
 ************************************************************************/

#ifndef TIMER_MANAGER_H_
#define TIMER_MANAGER_H_
#include<stdint.h>
#include<queue>
#include<vector>
#include<sys/time.h>
#include<pthread.h>
class Epoll;
typedef void (* TimerCallBack)(void*);

void* EpollFunc(void* param);

class TimerManager{
public:
    static TimerManager* InitTimerManager();
    ~TimerManager();
    //register timer, namely add TimerElement to timer_heap
    void RegisterTimer(uint32_t timeout,/*timeout in ms*/ TimerCallBack callback_func, void* param, bool once);

    //unregister timer, namely delete TimerElement from timer_heap, use TimerCallBack as the index, to find the TimerElement. 
    void UnRegisterTimer(TimerCallBack);  

    //start timer manager, epoll loop should be set in a indepedent thread
    void StartTimerMgr();

    void StopTimerMgr();
   
    //use epoll as timer engine
    void EpollLoop();
private:
    TimerManager();

    int TimeSpan(const struct timeval& time1, const struct timeval& time2);

    static TimerManager* timer_manager_;

    struct TimerElement{
        bool trigger_once;
        uint32_t time_span;
        struct timeval expire_time; //the time that this timer expires
        TimerCallBack callback;
        void* self_data;

        TimerElement(uint32_t time_sp,  TimerCallBack p, void* param, bool once):
            time_span(time_sp), callback(p), self_data(param), trigger_once(once){
                gettimeofday(&expire_time, NULL);
                DelayTimer();
        };
        void DelayTimer(){
            expire_time.tv_sec += time_span / 1000;
            expire_time.tv_usec += time_span % 1000;
        }
    };

    struct TimerCmpFunc{
        bool operator() (void* a, void* b){
            TimerElement* t1 = (TimerElement*)a;
            TimerElement* t2 = (TimerElement*)b;
            return (1000000* t1->expire_time.tv_sec + t1->expire_time.tv_usec)
                > (1000000 * t2->expire_time.tv_sec + t2->expire_time.tv_usec);
        }  
    };
    bool work_over_;

    pthread_t epoll_thread_;
    Epoll* epoll_;
    std::priority_queue<TimerElement*, std::vector<TimerElement*>, TimerCmpFunc> timer_heap_;
};
#endif
