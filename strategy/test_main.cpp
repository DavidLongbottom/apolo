
#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <typeinfo>
#include <queue>


#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>


#include <ta-lib/ta_libc.h>

#include "../util/util_func.h"
#include "macd.h"








int main(){

    std::ifstream f;
    f.open("close.csv", std::ios::in);
    if(!f.is_open()){
        std::cout << "open failed" << std::endl;
        return -1;
    }

    std::string line;
    int count = 0;
    
    std::vector<std::string> v;
    std::queue<double> close_q;

    while(std::getline(f, line)){
        if(count == 0) {
            count = count + 1;
            continue;
        }
        boost::algorithm::split(v, line, boost::is_any_of(","));
        std::cout << v[0] << "  " << v[1] << "  " << v[2] << std::endl;
        v.clear();
        close_q.push(boost::lexical_cast<double>(v[2]));
    }
    int SIZE = close_q.size();
    std::vector<double> close;
    for(int i = 0; i < SIZE ; i++){
        close.push_back(close_q.front());
        close_q.pop();
    }

    for( int i = 0; i < SIZE; i++){
        int a = macd_calulate(close, 0, i);
        std::cout << "i:  " << i << "  "<< a << std::endl;
    }
    std::vector<double> a;
    load_history(a);
    std::cout << a[0] <<"   " <<  a[1] << std::endl;

}


