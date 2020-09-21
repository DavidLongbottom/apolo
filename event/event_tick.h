#ifndef EVENT_TICK_H
#define EVENT_TICK_H

#include "event.h"


#include "../api/ctp/ThostFtdcMdApi.h"
#include "../api/ctp/ThostFtdcTraderApi.h"
#include "../api/ctp/ThostFtdcUserApiDataType.h"
#include "../api/ctp/ThostFtdcUserApiStruct.h"



class event_tick : public event{

private:
    std::string _type; 
    CThostFtdcDepthMarketDataField * _data;

public:
    event_tick(std::string type, CThostFtdcDepthMarketDataField * data);
    CThostFtdcDepthMarketDataField * get_event_data();
};


#endif