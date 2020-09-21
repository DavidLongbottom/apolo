#ifndef CONFIG_PROCESSOR_H
#define CONFIG_PROCESSOR_H

#include <string>
#include <map>





class config_processor{
    std::map<std::string, std::string> config_map;

public:
    config_processor(std::string filename);
    std::string get_value(std::string key);


};


#endif