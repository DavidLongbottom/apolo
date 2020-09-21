#ifndef ZMQ_SOCKET_FACTORY_H
#define ZMQ_SOCKET_FACTORY_H

#include <zmq.h>
#include <memory>
#include <iostream>

#include <assert.h>
#include <unistd.h>
#include <string.h>






class zmq_socket_factory{

    //param, socket_addr, is_server
    //这是判断一个socket惟一的依据（因为都采用了pub, sub）， 所以唯一的问题是看这个单例是否已经存在

public:
    

    static void * get_socket(void * context, std::string socket_addr, bool is_server, std::string optval =""){
       
        socket_addr = get_ipc_path(socket_addr);

        void * socket;
        if(is_server){
            socket = zmq_socket(context, ZMQ_PUB);
            int rc = zmq_bind(socket, socket_addr.c_str());
            assert(rc == 0);
        }else{
            socket = zmq_socket(context, ZMQ_SUB);
            int rc = zmq_connect(socket, socket_addr.c_str());
            assert(rc == 0);
            std::cout << "lala" << std::endl;
            
            //zmq_setsockopt(socket,ZMQ_SUBSCRIBE, optval.c_str(), sizeof(optval));
            if(optval ==""){
                zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);
            }else{
                //std::cout << optval.c_str() << "   hahaha" << std::endl;
                zmq_setsockopt(socket,ZMQ_SUBSCRIBE, optval.c_str(), strlen(optval.c_str()));
            }
            
        }
        return socket;
        

    }


    static void add_subscribe_key(void * socket, std::string optval =""){
            if(optval ==""){
                zmq_setsockopt(socket, ZMQ_SUBSCRIBE, "", 0);
            }else{
                //std::cout << optval.c_str() << "   hahaha" << std::endl;
                zmq_setsockopt(socket,ZMQ_SUBSCRIBE, optval.c_str(), strlen(optval.c_str()));
            }
    }      

    static void * close_socket(void * socket){
        zmq_close(socket);
    }

private:


    static std::string get_ipc_path(std::string ipc_name){
        return "ipc:///var/tmp/" + ipc_name +".cassiniipc";
    }

};
#endif