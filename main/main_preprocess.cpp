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


void send_bar(bar_generator& bg, std::pair<std::string, CThostFtdcDepthMarketDataField *>& pa, void * socket_bar ){
    kbar mbar = bg.on_tick(pa.second);

    if(mbar.type == 1) 
        z_msg_send(socket_bar, pa.second->InstrumentID, mbar);

    mbar = bg.on_five_bar(mbar);
    if( mbar.type == 5) 
        z_msg_send(socket_bar, pa.second->InstrumentID, mbar);

    mbar = bg.on_fifteen_bar(mbar);
    if(mbar.type == 15) 
        z_msg_send(socket_bar, pa.second->InstrumentID, mbar);

    mbar = bg.on_thirty_bar(mbar);
    if(mbar.type == 30) 
        z_msg_send(socket_bar, pa.second->InstrumentID, mbar);

    mbar = bg.on_hour_bar(mbar);
    if(mbar.type == 60) 
        z_msg_send(socket_bar, pa.second->InstrumentID, mbar);

}



// 这个进程目的是为了选择订阅从main_md 发出来的某个或者某些tick， 在这里可以做简单的处理， 比如说生成k 线的数据 
// 这个进程把处理后的数据进一步广播出去， 这样做的场景是比如说对一个期货， 同时运行1000个策略， 不广播的话在一个进程上处理是不行的
int main(){
    
    zmq_context_singleton & singleton = zmq_context_singleton::singleton();
    void * context = singleton.get_singleton_context();

    void * socket_md = zmq_socket_factory::get_socket(context, "tick", 0);
    void * socket_bar = zmq_socket_factory::get_socket(context, "bar", 1);


    zmq_pollitem_t items[] ={
        {socket_md, 0, ZMQ_POLLIN, 0}
    };


    bar_generator bg;


    int i = 0;
    while(i < 30){
        
        zmq_poll(items, 1, -1);

        if(items[0].revents & ZMQ_POLLIN){
            std::pair<std::string, CThostFtdcDepthMarketDataField *> pa = z_msg_recv<CThostFtdcDepthMarketDataField>(socket_md);    
            send_bar(bg, pa, socket_bar);

        }
        
    }

    zmq_socket_factory::close_socket(socket_md);
    zmq_socket_factory::close_socket(socket_bar);
    singleton.close_context();



}







