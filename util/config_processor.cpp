#include "config_processor.h"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <filesystem>
#include <boost/algorithm/string.hpp>



std::string get_app_path(){
    int LENGTH = 255;
    char path[LENGTH];
    if(getcwd(path, LENGTH) != NULL){
        std::string p = path;
        //std::filesystem::path ab(p);
        //p = ab.parent_path(); 

        return p;
    }
    else{
        return nullptr;
    }
};







config_processor::config_processor(std::string filename){
    std::string path = get_app_path();
    std::ifstream f;
    f.open(path + "/../config/" + filename, std::ios::in);
    std::cout << path + "/../config/" + filename << std::endl;
    assert(f.is_open());

    std::string line;
    std::string key;
    std::string value;
    while(std::getline(f, line)){
        boost::algorithm::trim(line);
        if(line.length() == 0) continue;
        if(line[0] == '#') continue;
        
        int pos_equal = line.find('=');
        key = line.substr(0, pos_equal);
        boost::algorithm::trim(key);
        value = line.substr(pos_equal+1);
        boost::algorithm::trim(value);
        config_map[key] = value;
    }

}


std::string config_processor::get_value(std::string key){
    std::map<std::string, std::string>::iterator ite = config_map.find(key);
    if(ite == config_map.end()) return "";
    return ite ->second;
}