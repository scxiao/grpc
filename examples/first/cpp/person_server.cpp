#include <iostream>
#include <memory>
#include <string>
#include <fstream>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "addressbook.pb.h"
#include "addressbook.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerReader;
using grpc::ServerWriter;
using tutorial::Greeter;
using tutorial::Person;
using tutorial::PersonRequest;

void CreatePersonInfo(const tutorial::AddressBook& addr_book, const std::string& id, Person *person_info)
{
    assert(person_info != nullptr);
    for (int i = 0; i < addr_book.people_size(); ++i)
    {
        const auto& psn = addr_book.people(i);
        const std::string& name = psn.name();
        if (name == id)
        {
            *person_info = psn;
            return;
        }
    }
}

class GetPersonServiceImpl final : public Greeter::Service
{
    Status GetPerson(ServerContext *context, const PersonRequest *request,
                     Person *reply) override 
    {
        std::fstream ifile("addr1", std::ios::binary | std::ios::in);
        tutorial::AddressBook addr_book;
        if (not addr_book.ParseFromIstream(&ifile))
        {
            std::cerr << "Failed to parse address book \"addr\"!" << std::endl;
            return Status::CANCELLED;
        }
        std::string name = request->name();
        std::cout << "request_name = " << name << std::endl;
        CreatePersonInfo(addr_book, name, reply);

        return Status::OK;
    }

    Status ListPersons(ServerContext* context, 
                       const PersonRequest* request, 
                       ServerWriter<Person>* writer) override 
    {
        std::fstream ifile("addr1", std::ios::binary | std::ios::in);
        tutorial::AddressBook addr_book;
        if (not addr_book.ParseFromIstream(&ifile))
        {
            std::cerr << "Failed to parse address book \"addr\"!" << std::endl;
            return Status::CANCELLED;
        }
        std::string name = request->name();
        std::cout << "request_name = " << name << std::endl;
        if (name == "all")
        {
            for (int i = 0; i < addr_book.people_size(); ++i)
            {
                const auto& p = addr_book.people(i);
                writer->Write(p);
            }
        }
        else
        {
            Person p;
            CreatePersonInfo(addr_book, name, &p);
            writer->Write(p);
        }

        return Status::OK;
    }

    Status recordPerson(ServerContext* context, ServerReader<tutorial::storeInfo>* reader,
                        tutorial::storeConfirmation* confirm) override 
    {
        tutorial::storeInfo sinfo;
        std::string file_name;
        tutorial::AddressBook addr_book;
        while(reader->Read(&sinfo))
        {
            file_name = sinfo.file_name();
            tutorial::Person* person = addr_book.add_people();
            *person = sinfo.psn();
        }

        if (not file_name.empty())
        {
            std::fstream output(file_name, std::ios::out | 
                                           std::ios::trunc | 
                                           std::ios::binary);
            if (not addr_book.SerializeToOstream(&output))
            {
                std::cerr << "Serialize to output file error" << std::endl;
            }
        }

        return Status::OK;
    }
};

void RunServer()
{
    std::string server_address("0.0.0.0:50051");
    GetPersonServiceImpl service;
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}

int main()
{
    RunServer();

    return 0;
}
