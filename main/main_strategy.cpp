#include <fstream>
#include <assert.h>


#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "../ipc/zmq_ipc/zmq_context_singleton.hpp"
#include "../ipc/zmq_ipc/zmq_socket_factory.hpp"
#include "../ipc/zmq_ipc/zmqhelper.hpp"

#include "../api/ctp/ThostFtdcMdApi.h"
#include "../api/ctp/ThostFtdcTraderApi.h"
#include "../api/ctp/ThostFtdcUserApiDataType.h"
#include "../api/ctp/ThostFtdcUserApiStruct.h"

#include "../event/event_engine.h"
#include "../event/event_tick.h"

#include "../pojo/market_data.h"
#include "../pojo/bar.h"
#include "../util/bar_generator.h"
#include "../strategy/macd.h"
#include "../util/util_func.h"



// 这个文件对应的进程是为了从main_preprocess 里接收预处理数据， 进行策略的生成
int main(){
    
    zmq_context_singleton & singleton = zmq_context_singleton::singleton();
    void * context = singleton.get_singleton_context();

    void * socket_bar = zmq_socket_factory::get_socket(context, "bar", 0);
    void * socket_suggestion = zmq_socket_factory::get_socket(context, "suggestion", 1);


    zmq_pollitem_t items[] ={
        {socket_bar, 0, ZMQ_POLLIN, 0}
    };


    int load_history_flag = 0;
    std::vector<double> v;

    int i = 0;
    while(i < 30){
        
        zmq_poll(items, 1, -1);

        if(items[0].revents & ZMQ_POLLIN){

            if(load_history_flag == 0){
                load_history(v);
            }
            std::pair<std::string, kbar *> pa = z_msg_recv<kbar>(socket_bar);    
            std::cout << pa.second->contract << "     ";
            std::cout << pa.second->minute << "     ";
            std::cout << pa.second->high << "     ";
            std::cout << pa.second->low << "     ";
            std::cout << pa.second->open << "     ";
            std::cout << pa.second->close << std::endl;

            v.push_back(pa.second->close);
            int result = macd_calulate(v, v.size() - 40, v.size()-1);
            if(result == -2) continue;

            //other strategy calculation  like 100 kind 
            //combine several strategy to get the suggestion

            //order suggestion send out 
            z_msg_send(socket_suggestion, pa.second->contract, result);

        }
        
    }

    zmq_socket_factory::close_socket(socket_bar);
    singleton.close_context();



}







