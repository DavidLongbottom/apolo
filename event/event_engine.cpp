#include "event_engine.h"



event_engine::event_engine(){
    std::cout << "engine init" << std::endl;
}

event_engine::~event_engine(){
    std::cout << "engine stop" << std::endl;
}

void event_engine::start_engine(){
    consumer_thread = new std::thread(std::bind(&event_engine::process_event, this));
    //consumer_thread->detach();
    std::cout << "engine start" << std::endl;
}


bool event_engine::register_event(std::string type, handler_func handler){
    std::map<std::string, handler_func_list>::iterator ite = relation_map.find(type);
    if(ite == relation_map.end()){
        std::list<handler_func> hlist;
        hlist.push_back(handler);
        relation_map.insert(std::make_pair(type, hlist));
    }else{
        ite -> second.push_back(handler);
    }
    return true;
}

bool event_engine::unregister_event(std::string type){
    relation_map.erase(type);
    return true;
}

bool event_engine::put_event(event * e){
    this->syn_q.push(e);
    return true;
}


void event_engine::process_event(){
    while(true){
        event * e = this->syn_q.get();
        handler_func_list func_list = relation_map[e->get_event_type()];
        for(handler_func func : func_list){
            func(e);
        }
    }
}




