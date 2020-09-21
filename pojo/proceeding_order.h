#ifndef PROCEEDING_ORDER_H
#define PROCEEDING_ORDER_H

#include <string>


struct proceeding_order
{
    int status;   // 0  初始化； 1 正在进行； 2 结束； -1 错误；
    std::string instrument_id;
    std::string order_type;
    int direction;
    int init_price;
    int init_volume;
    int complete_volume;
    int left_volume;
     
};




#endif