#include  "event_order_status.h"




event_order_status::event_order_status(std::string type, CThostFtdcInputOrderField order_field) : event(type){
    this ->_type = type;
    this ->_order_field = order_field;
}

CThostFtdcInputOrderField event_order_status::get_event_data(){
    return this->_order_field;
}

