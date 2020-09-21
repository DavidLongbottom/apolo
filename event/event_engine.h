#ifndef EVENT_ENGINE_H
#define EVENT_ENGINE_H

#include <map>
#include <functional>
#include <thread>
#include <list>

#include "event.h"
#include "sync_queue.hpp"

typedef std::function<void(event *)> handler_func;
typedef std::list<handler_func> handler_func_list;

// 把event 放入一个线程安全的队列，然后从队列中取出来。 根据保存的信息调用对应的回调函数
class event_engine{

private:
    
    //线程安全队列
    sync_queue<event *> syn_q;

    //event 与回调函数的对应的map
    std::map<std::string, handler_func_list> relation_map;

    // 需要一个消费者的线程（多生产者，单一消费）
    std::thread * consumer_thread;


public:
    event_engine();

    ~event_engine();
    
    //向map中添加对应关系
    bool register_event(std::string, handler_func);

    //向map中注销对应关系
    bool unregister_event(std::string);

    bool put_event(event *);

    //void process_event();

    //获取map的对应关系
    //handler_func_list get_event_handler(std::string);

    void start_engine();

    void process_event();

};


#endif