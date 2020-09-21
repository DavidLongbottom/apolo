#include "event_trade_status.h"


event_trade_status::event_trade_status(std::string type, CThostFtdcTradeField * trade_field): event(type){
    this -> _type = type;
    this -> _trade_field = trade_field;
}

CThostFtdcTradeField * event_trade_status::get_event_data(){
    return this -> _trade_field;
}

