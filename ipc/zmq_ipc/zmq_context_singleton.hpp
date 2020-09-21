#ifndef ZMQ_CONTEXT_SINGLETON_HPP
#define ZMQ_CONTEXT_SINGLETON_HPP

#include <zmq.h>
#include <memory>
#include <iostream>
#include <mutex>

//Meyers' Singleton Meyer's的单例, z这里把get_context_instance里面的内容进行了稍微的改变
//https://stackoverflow.com/questions/14331469/undefined-reference-to-static-variable

// 由于context 在一个进程中是单例，所以销毁应该是自动销毁的， 而不需要人手动销毁。

class zmq_context_singleton{

public:

    static zmq_context_singleton & singleton(){
        static zmq_context_singleton context;
        return context;
    }

    void * get_singleton_context(){
        std::lock_guard<std::mutex> locker(mtx);
        if(zmq_context == nullptr){
            zmq_context = zmq_ctx_new();
        }
        return zmq_context;
    }

    void  close_context(){
        if(zmq_context != nullptr){
            zmq_ctx_destroy(zmq_context);
        }
    }

private:

    void * zmq_context = nullptr;
    std::mutex mtx;

};

#endif
