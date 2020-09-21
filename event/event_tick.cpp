#include "event_tick.h"



event_tick::event_tick(std::string type, CThostFtdcDepthMarketDataField * data) : event(type){
    this -> _type = type;
    this -> _data = data;
}

CThostFtdcDepthMarketDataField * event_tick::get_event_data(){
    return this -> _data;
}
