#ifndef EVENT_H
#define EVENT_H

#include <iostream>


class event {

private:
    std::string _type; 

public:
    event(std::string type);
    std::string get_event_type();
    
};



#endif











