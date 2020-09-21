#include <iostream>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/lexical_cast.hpp>


#include "bar_generator.h"


bool bar_generator::time_filter(CThostFtdcDepthMarketDataField * md){
    //TODO
    return true;    
}

kbar bar_generator::on_tick(CThostFtdcDepthMarketDataField * md){
    kbar ret;

    boost::gregorian::date d = boost::gregorian::from_undelimited_string( md -> TradingDay);
    std::string time_s(md -> UpdateTime);
    std::string temp_str = "1900-01-01 " + time_s;
    boost::posix_time::ptime temp_dt = boost::posix_time::time_from_string(temp_str);
    boost::posix_time::ptime dt(d, temp_dt.time_of_day());
/*
    std::string key = boost::lexical_cast<std::string>(dt.time_of_day().hours()) + ":" 
    + boost::lexical_cast<std::string>(dt.time_of_day().minutes()) + + ":00";
*/
    std::string key = md ->InstrumentID;
    std::map<std::string, kbar>::iterator ite;
    ite = bar_map_one.find(key);


    if(ite != bar_map_one.end()){
        if(ite -> second.minute != dt.time_of_day().minutes()){
            std::cout << ite->second.contract << "   hour: "<< ite->second.hour << "  minute:" << ite->second.minute 
            << "  high:" << ite->second.high << "  low:" << ite->second.low << "  start:"
            <<ite->second.open << "  close: "<< ite->second.close << std::endl;
            
            ret = ite->second;

            ite->second.hour = dt.time_of_day().hours();
            ite->second.minute = dt.time_of_day().minutes();
            ite->second.second = 0;
            ite->second.high = boost::lexical_cast<float>(md->LastPrice);
            ite->second.low = boost::lexical_cast<float>(md->LastPrice);
            ite->second.open = boost::lexical_cast<float>(md->LastPrice);
            ite->second.close = boost::lexical_cast<float>(md->LastPrice);
        }else{
            if(boost::lexical_cast<float>(md ->LastPrice) > ite->second.high)
                ite->second.high = boost::lexical_cast<float>(md ->LastPrice);
            
            if(boost::lexical_cast<float>(md ->LastPrice) < ite->second.low)
                ite->second.low = boost::lexical_cast<float>(md ->LastPrice);

        }

    }
    else{
        kbar b;
        strcpy(b.contract, md ->InstrumentID);
        strcpy(b.day, md->TradingDay);
        b.hour = dt.time_of_day().hours();
        b.minute = dt.time_of_day().minutes();
        b.second = 0;
        b.open = boost::lexical_cast<float>(md->LastPrice);
        b.high = boost::lexical_cast<float>(md->LastPrice);
        b.low = boost::lexical_cast<float>(md->LastPrice);
        b.type = 1;

        bar_map_one.insert(std::make_pair(md->InstrumentID, b));

    }

    return ret;

}


 kbar bar_generator::on_five_bar(kbar b){
    //TODO 
}

kbar bar_generator::on_fifteen_bar(kbar b){
    //TODO
}

kbar bar_generator::on_thirty_bar(kbar b){
    //TODO
}

kbar bar_generator::on_hour_bar(kbar b){
    //TODO
}