#ifndef EVENT_ORDER_STATUS_H
#define EVENT_ORDER_STATUS_H

#include "event.h"

#include "../api/ctp/ThostFtdcMdApi.h"
#include "../api/ctp/ThostFtdcTraderApi.h"
#include "../api/ctp/ThostFtdcUserApiDataType.h"
#include "../api/ctp/ThostFtdcUserApiStruct.h"



class event_order_status : public event{

private:
    std::string _type;
    CThostFtdcInputOrderField _order_field;

public:
    event_order_status(std::string type, CThostFtdcInputOrderField order_field);
    CThostFtdcInputOrderField get_event_data();

};


#endif