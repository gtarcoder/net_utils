/*************************************************************************
	> File Name: timer_manager.cpp
	> Author   : JasonSun 
	> Mail     : skc361@163.com 
	> Created Time: 2015年04月07日 星期二 18时39分38秒
 ************************************************************************/
#include<stdio.h>
#include<unistd.h>
#include"timer_manager.h"
#include"epoll_wrapper.h"
void* EpollFunc(void* param){
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    TimerManager* timer_mgr = TimerManager::InitTimerManager();
    timer_mgr->EpollLoop();
    return (void*)0;
}

TimerManager* TimerManager::timer_manager_ = NULL;

TimerManager::TimerManager():work_over_(true){
    epoll_ = new Epoll();
    epoll_->AddEvent(STDOUT_FILENO, EPOLLIN);
    //use stdin file description, and wait its epoll_in event, obviously, this kind of event will never occurs

    StartTimerMgr();
}

TimerManager::~TimerManager(){
    delete epoll_;
}

TimerManager* TimerManager::InitTimerManager(){
    if (timer_manager_ == NULL){
        timer_manager_ = new TimerManager();
    }
    return timer_manager_;
}

void TimerManager::RegisterTimer(uint32_t time_out, TimerCallBack p, void* param, bool once){
    TimerElement* timer_element = new TimerElement(time_out, p, param, once);
    timer_heap_.push(timer_element); 
}

void TimerManager::UnRegisterTimer(TimerCallBack p){
    std::queue<TimerElement*> timerQ;
    TimerElement* timer_elmt;
    while(! timer_heap_.empty()){
        timer_elmt = timer_heap_.top();
        timer_heap_.pop();
        if (timer_elmt->callback == p){
            delete timer_elmt; //remember to release the memory 
            break;
        }else{
            timerQ.push(timer_elmt);
        }
    }

    while(! timerQ.empty()){
        timer_elmt = timerQ.front();
        timer_heap_.push(timer_elmt);
        timerQ.pop();
    }
}

void TimerManager::EpollLoop(){
    TimerElement* timer_elmt;
    struct timeval cur_time;
    while(! work_over_){
        if (timer_heap_.empty()){
            usleep(1000);
            continue;
        }

        timer_elmt = timer_heap_.top();
        timer_heap_.pop();
        gettimeofday(&cur_time, NULL);
        epoll_->Wait(TimeSpan(timer_elmt->expire_time, cur_time));
        //call the timer callback
        (*(timer_elmt->callback))(timer_elmt->self_data);
        

        if (! timer_elmt->trigger_once) {
           timer_elmt->DelayTimer();
           timer_heap_.push(timer_elmt);
        }else{
            delete timer_elmt;
        }
    }
}

int TimerManager::TimeSpan(const struct timeval& time1, const struct timeval& time2){
    int result = (time1.tv_sec - time2.tv_sec)*1000 + (time1.tv_usec - time2.tv_usec) / 1000;
    return result > 0?  result : 0;
}

void TimerManager::StartTimerMgr(){
    if (! work_over_){
        return;
    }
    work_over_ = false;
    if ( pthread_create(&epoll_thread_, NULL, EpollFunc, NULL) != 0){
        perror("create timer manager thread\n");
    }
}

void TimerManager::StopTimerMgr(){
    if (work_over_){
        return;
    }
    work_over_ = true;
}
