#ifndef __PERSON_HPP__
#define __PERSON_HPP__

#include <string>

struct PersonInfo
{
    std::string name;
    int32_t id;
    std::string email;
    std::string phone_num;

    PersonInfo() 
    {
        name = "";
        id = -1;
        email = "";
        phone_num = "";
    }
};

#endif
