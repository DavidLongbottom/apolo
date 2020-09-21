#ifndef ZMQHELPER_HPP
#define ZMQHELPER_HPP

#include <string.h>
#include <malloc.h>
#include <zmq.h>
#include <string>


//todo, 这样实现的话其实很不优雅， 需要客户端知道服务端的类型， 并且事实上这个并没有具体的更进一步的封装成event
template <typename T>
inline int z_msg_send(void * socket, std::string key, T value){
    zmq_msg_t key_msg;
    zmq_msg_t value_msg;

    // 注意strlen 与 sizeof 的区别
    zmq_msg_init_size(& key_msg, strlen(key.c_str()));
    memcpy(zmq_msg_data(& key_msg), key.c_str(), strlen(key.c_str()));
    zmq_msg_send(&key_msg, socket, ZMQ_SNDMORE);

    zmq_msg_init_size(& value_msg, sizeof(value));
    memcpy(zmq_msg_data(& value_msg), & value, sizeof(value));
    zmq_msg_send(&value_msg, socket, 0);

    zmq_msg_close(&key_msg);
    zmq_msg_close(&value_msg);
    //std::cout << "template" <<std::endl;


}

template <typename T>
inline std::pair<std::string, T *> z_msg_recv(void * socket){
    zmq_msg_t key_msg;
    zmq_msg_init(& key_msg);

    int size = zmq_msg_recv(& key_msg,socket,0);
    char * msg_key = (char *)malloc(size+1);
    memcpy(msg_key,zmq_msg_data(& key_msg),size);
    zmq_msg_close(& key_msg);
    // 这至关重要， 这把字符串的最后一位的后一位设置为0， c语言的char * 会读到0结束，这样确保字符串不会多读
    msg_key[size] = 0;  
    std::string s = msg_key;


    zmq_msg_t message;
    zmq_msg_init(& message);
    int rc = zmq_msg_recv(& message,socket,0);
    T * msg_value = (T *)malloc(rc+1);
    memcpy(msg_value,zmq_msg_data(& message),rc);
    zmq_msg_close(& message);

    std::pair<std::string, T *> pa(s, msg_value);

    free(msg_key);
    return pa;

}



//////////////////////////
//模板特化
template <>
inline int z_msg_send<std::string>(void * socket, std::string key, std::string value){
    zmq_msg_t key_msg;
    zmq_msg_t value_msg;

    // 注意strlen 与 sizeof 的区别
    zmq_msg_init_size(& key_msg, strlen(key.c_str()));
    memcpy(zmq_msg_data(& key_msg), key.c_str(), strlen(key.c_str()));
    zmq_msg_send(&key_msg, socket, ZMQ_SNDMORE);

    zmq_msg_init_size(& value_msg, strlen(value.c_str()));
    memcpy(zmq_msg_data(& value_msg), value.c_str(), strlen(value.c_str()));
    zmq_msg_send(&value_msg, socket, 0);

    zmq_msg_close(&key_msg);
    zmq_msg_close(&value_msg);
    //std::cout << "specify" <<std::endl;


}

//模板特化
inline std::pair<std::string, std::string> z_msg_recv(void * socket){
    zmq_msg_t key_msg;
    zmq_msg_init(& key_msg);

    int size = zmq_msg_recv(& key_msg,socket,0);
    char * msg_key = (char *)malloc(size+1);
    memcpy(msg_key,zmq_msg_data(& key_msg),size);
    zmq_msg_close(& key_msg);
    // 这至关重要， 这把字符串的最后一位的后一位设置为0， c语言的char * 会读到0结束，这样确保字符串不会多读
    msg_key[size] = 0;  
    std::string s = msg_key;


    zmq_msg_t message;
    zmq_msg_init(& message);
    int rc = zmq_msg_recv(& message,socket,0);
    char * msg_value = (char *)malloc(rc+1);
    memcpy(msg_value,zmq_msg_data(& message),rc);
    zmq_msg_close(& message);
    msg_value[rc] = 0;
    std::string s_v = msg_value;

    std::pair<std::string, std::string> pa(s, s_v);

    free(msg_key);
    free(msg_value);
    return pa;

}

#endif
