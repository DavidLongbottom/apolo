#include "util_func.h"


void s2t(){
    tm tm_;
    time_t t_;
    char buf[128]= {0};

    strcpy(buf, "2012-01-01 14:00:00");
    strptime(buf, "%Y-%m-%d %H:%M:%S", &tm_); //将字符串转换为tm时间
    tm_.tm_isdst = -1;
    t_  = mktime(&tm_); //将tm时间转换为秒时间
    t_ += 3600;  //秒数加3600

    tm_ = *localtime(&t_);//输出时间
    strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &tm_);
    std::cout << buf << std::endl;

}


void load_history(std::vector<double> & v_double){
        //load history data
    //首先要有一个建立的数据库之类的东西，专门存储k bar，然后当异常发生终止的时候， 当重新加载的时候需要先进行数据加载
    // 此仅仅为示例
    std::ifstream f;
    f.open("close.csv", std::ios::in);
    assert(f.is_open());
    std::string line;
    std::vector<std::string> v;
    std::queue<double> q;
    int count = 0;
    while(std::getline(f, line)){
        if(count == 0){
            count = count + 1;
            continue;
        }
        boost::algorithm::split(v, line, boost::is_any_of(","));
        q.push(boost::lexical_cast<double>(v[2]));
        v.clear();
    }
    int size = q.size();

    for(int i = 0; i < size; i++){
        v_double.push_back(q.front());
        q.pop();
    }

}

