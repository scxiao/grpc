#ifndef __PERSON_HPP__
#define __PERSON_HPP__

#include <string>

struct PersonInfo
{
    std::string name;
    int32_t id;
    std::string email;
    std::string phone_num;
    std::string phone_type;
    // std::unordered_map<std::string, std::string> phone_nums;

    PersonInfo() 
    {
        name = "";
        id = -1;
        email = "";
        phone_num = "";
        phone_type = "";
    }
};

#endif
