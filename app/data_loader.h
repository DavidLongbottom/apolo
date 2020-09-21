#ifndef DATA_LOADER_H
#define DATA_LOADER_H

//事实上定义data_loader 是没有任何价值和意义的， 因为处理数据的时候可能会从不同的地方进行加载，
// 而且数据的格式可能也是不同的
// 唯一有用的是已经有大量的数据库， 有固定的格式， 然后写一个data_loader 的类来进行数据的加载
class data_loader{
    
    public:
        bool load_from_file();
        bool load_from_net();

};



#endif