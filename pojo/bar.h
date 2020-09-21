#ifndef BAR_H
#define BAR_H

struct kbar{
    char  contract[31];
    char  day[15];
    int   hour;
    int   minute;
    int   second;
    int   type;
    float open;
    float high;
    float low;
    float close;
};


#endif