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
using grpc::ClientReader;
using grpc::ClientWriter;
using grpc::ClientReaderWriter;
using grpc::Status;
using tutorial::Greeter;
using tutorial::Person;
using tutorial::PersonRequest;

PersonInfo convert_from_person(const tutorial::Person& psn)
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


std::unordered_map<std::string, tutorial::Person_PhoneType> map_phone_type = 
{
    {"mobile", tutorial::Person::MOBILE},
    {"home", tutorial::Person::HOME},
    {"work", tutorial::Person::WORK}
};

tutorial::Person convert_to_person(const PersonInfo& info)
{
    tutorial::Person psn;
    psn.set_id(info.id);
    psn.set_name(info.name);
    psn.set_email(info.email);

    tutorial::Person::PhoneNumber* phone_num = psn.add_phones();
    phone_num->set_number(info.phone_num);
    if (map_phone_type.count(info.phone_type))
    {
        phone_num->set_type(map_phone_type.at(info.phone_type));
    }
    else
    {
        phone_num->set_type(tutorial::Person::MOBILE);
    }

    return psn;
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
    
    PersonInfo GetPerson(const std::string& name)
    {
        PersonRequest request;
        Person reply;
        request.set_name(name);

        ClientContext context;
        Status status = stub_->GetPerson(&context, request, &reply);

        if (status.ok())
        {
            PersonInfo info = convert_from_person(reply);
            return info;
        }
        else
        {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            std::abort();
        }
    }

    std::vector<PersonInfo> ListPersons(const std::string& name)
    {
        PersonRequest request;
        request.set_name(name);

        ClientContext context;
        std::unique_ptr<ClientReader<Person> > reader(stub_->ListPersons(&context, request));

        std::vector<PersonInfo> result;
        Person p;
        while (reader->Read(&p))
        {
            PersonInfo info = convert_from_person(p);
            result.push_back(info);
        }
        Status status = reader->Finish();
        if (status.ok()) {
            std::cout << "ListFeatures rpc succeeded." << std::endl;
        } else {
            std::cout << "ListFeatures rpc failed." << std::endl;
        }

        return result;
    }

    void recordPerson(const std::string& filename, const std::vector<PersonInfo>& vec_psns)
    {
        tutorial::storeInfo sinfo;
        tutorial::storeConfirmation confirm;
        ClientContext context;
        std::unique_ptr<ClientWriter<tutorial::storeInfo>> writer(stub_->recordPerson(&context, &confirm));
        std::string file_name(filename);

        for (std::size_t i = 0; i < vec_psns.size(); ++i)
        {
            // convert PersonInfo to storeInfo struct
            tutorial::Person psn = convert_to_person(vec_psns.at(i));
            sinfo.set_allocated_psn(&psn);
            sinfo.set_allocated_file_name(&file_name);
            if (not writer->Write(sinfo))
            {
                break;
            }
        }
        writer->WritesDone();
        Status status = writer->Finish();
        if (status.ok())
        {
            std::cout << "Finished sending person information" << std::endl;
        }
        else
        {
            std::cout << "recordPerson rpc failed!" << std::endl;
        }
    }

  private:
    std::unique_ptr<Greeter::Stub> stub_;
};

std::string get_option_value(const std::string& option, const std::vector<std::string>& strs)
{
    for (auto& str : strs)
    {
        auto pos = str.find(option);
        if (pos == 0)
        {
            pos += option.size();
            if (str[pos] != '=')
            {
                std::cout << "Incorrect option format for \"" << str << "\"" << std::endl;
                return {};
            }
            else
            {
                return str.substr(pos + 1);
            }
        }
        return {};
    }
}

int main(int argc, char **argv)
{
    std::string target = "localhost:50051";
    std::string name = "all";
    if (argc > 1)
    {
        std::vector<std::string> options;
        for (int i = 1; i < argc; ++i)
        {
            options.push_back({argv[i]});
        }

        auto val = get_option_value("--target", options);
        if (not val.empty())
        {
            target = val;
        }

        val = get_option_value("--name", options);
        if (not val.empty())
        {
            name = val;
        }
    }

    PersonClient pc(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));
    auto persons = pc.ListPersons(name);

    std::cout << "Received person info:" << std::endl;
    for (auto& info : persons)
    {
        print_person_info(info);
    }

    return 0;
}
