#ifndef MACD_H
#define MACD_H

#include <ta-lib/ta_libc.h>
#include <iostream>
#include <typeinfo>
#include <vector>


inline int macd_calulate(std::vector<double>& close, int start_id, int end_id){
    

    TA_RetCode retcode;
    retcode = TA_Initialize();
    if(retcode != TA_SUCCESS){
        return -2;
    }
    
    //int SIZE = sizeof(close)/sizeof(close[0]);
    
    int SIZE = end_id - start_id;


    double dif[SIZE];
    double dea[SIZE];
    double macd[SIZE];
    int out_begin;
    int out_nb;

    retcode = TA_MACD(start_id, end_id, &close[0], 12, 26, 9, &out_begin, &out_nb , &dif[0], &dea[0], &macd[0]);
    if(retcode != TA_SUCCESS){
        return -2;
    }

    double last_macd = macd[out_nb - 2];
    double last_2_macd = macd[out_nb - 3];
    if(last_macd * last_2_macd > 0) return 0;

    if(dif[out_nb - 2] < 0 && dea[out_nb-2] < 0 && last_macd < last_2_macd){
        return -1;
    }

    if(dif[out_nb - 2] > 0 && dea[out_nb-2] > 0 && last_macd > last_2_macd){
        return 1;
    }
    return 0;

}



#endif