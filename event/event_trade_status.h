#ifndef EVENT_TRADE_STATUS_H
#define EVENT_TRADE_STATUS_H

#include "event.h"


#include "../api/ctp/ThostFtdcMdApi.h"
#include "../api/ctp/ThostFtdcTraderApi.h"
#include "../api/ctp/ThostFtdcUserApiDataType.h"
#include "../api/ctp/ThostFtdcUserApiStruct.h"



class event_trade_status : public event{

private:
    std::string _type; 
    CThostFtdcTradeField * _trade_field;

public:

    CThostFtdcTradeField * get_event_data();
    event_trade_status(std::string type, CThostFtdcTradeField * trade_field);
};

#endif