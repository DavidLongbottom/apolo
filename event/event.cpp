#include "event.h"

event::event(std::string type){
    this->_type = type;
}

std::string event::get_event_type(){
    return this->_type;
}












