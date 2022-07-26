#include <iostream>
#include <memory>
#include <string>
#include <sstream>

#include <grpcpp/grpcpp.h>
#include "addressbook.pb.h"
#include "addressbook.grpc.pb.h"
#include "person.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using tutorial::Greeter;
using tutorial::Person;
using tutorial::PersonRequest;

    std::string name;
    int32_t id;
    std::string email;
    std::string phone_num;

PersonInfo convert_to_persion_info(const tutorial::Person& psn)
{
    PersonInfo info;
    info.name = psn.name();
    info.id = psn.id();
    info.email = psn.email();

    std::stringstream ss;
    for (int j = 0; j < psn.phones_size(); ++j)
    {
        const auto& pn = psn.phones(j);
        switch(pn.type())
        {
          case tutorial::Person::MOBILE:
            ss << " Mobile phone #: ";
            break;
          case tutorial::Person::HOME:
            ss << " Home phone #: ";
            break;
          case tutorial::Person::WORK:
            ss << " Work phone #: ";
            break;
        }
        ss << pn.number() << std::endl;
    }

    info.phone_num = ss.str();

    return info;
}

void print_person_info(const PersonInfo& info)
{
    std::cout << "Person ID: " << info.id << std::endl;
    std::cout << "  Name: " << info.name << std::endl;
    std::cout << "  E-mail address: " << info.email << std::endl;
    std::cout << "  " << info.phone_num << std::endl;
}

class PersonClient {
  public:
    PersonClient(std::shared_ptr<Channel> channel) : 
        stub_(Greeter::NewStub(channel)) {}
    
    PersonInfo GetInfo(const std::string& name)
    {
        PersonRequest request;
        Person reply;
        request.set_name(name);

        ClientContext context;
        Status status = stub_->GetInfo(&context, request, &reply);

        if (status.ok())
        {
            PersonInfo info = convert_to_persion_info(reply);
            return info;
        }
        else
        {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            std::abort();
        }
    }

  private:
    std::unique_ptr<Greeter::Stub> stub_;
};


int main(int argc, char **argv)
{
    std::string target_str;
    std::string arg_str("--target");
    if (argc > 1)
    {
        std::string arg_val = argv[1];
        auto start_pos = arg_val.find(arg_str);
        if (start_pos != std::string::npos)
        {
            start_pos += arg_str.size();
            if (arg_val[start_pos] == '=')
            {
                target_str = arg_val.substr(start_pos + 1);
            }
            else
            {
                std::cout << "The only correct argument syntax is --target=" << std::endl;
                return 1;
            }
        }
        else
        {
            std::cout << "The only correct argument syntax is --target=" << std::endl;
            return 1;
        }
    }
    else
    {
        target_str = "localhost:50051";
    }

    PersonClient pc(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    std::string name("abc");
    auto person_info = pc.GetInfo(name);

    std::cout << "Received person info:" << std::endl;
    print_person_info(person_info);

    return 0;
}
