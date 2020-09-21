#ifndef MARKET_DATA_H
#define MARKET_DATA_H
#include <stdio.h>
#include <sys/time.h>

struct market_data{
    char ticker[32];
    double bids[5];
    double asks[5];
    int bid_volume[5];
    int ask_volume[5];
    double last_trade;
    int last_trade_size;
    int volume;
    double turnover;
    double open_interest;
    bool is_trade_update;  // true if updated caused by trade
    timeval time;

    bool is_initialized;
    int count;
};
#endif














