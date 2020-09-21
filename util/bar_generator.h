#ifndef BAR_GENERATOR_H
#define BAR_GENERATOR_H

#include <map>
#include <iostream>
#include "../pojo/bar.h"


#include "../api/ctp/ThostFtdcMdApi.h"
#include "../api/ctp/ThostFtdcTraderApi.h"
#include "../api/ctp/ThostFtdcUserApiDataType.h"
#include "../api/ctp/ThostFtdcUserApiStruct.h"


class bar_generator{
    
    public:
        // key contract name, last time one minute kbar
        std::map<std::string, kbar> bar_map_one;

        std::map<std::string, kbar> bar_map_five;
        std::map<std::string, kbar> bar_map_fifteen;
        std::map<std::string, kbar> bar_map_thirty;
        std::map<std::string, kbar> bar_map_hour;



        bool time_filter(CThostFtdcDepthMarketDataField * md);

        kbar on_tick(CThostFtdcDepthMarketDataField * md);

        kbar on_five_bar(kbar b);

        kbar on_fifteen_bar(kbar b);

        kbar on_thirty_bar(kbar b);

        kbar on_hour_bar(kbar b);

};
#endif