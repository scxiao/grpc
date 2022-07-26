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

class GetInfoServiceImpl final : public Greeter::Service
{
    Status GetInfo(ServerContext *context, const PersonRequest *request,
                   Person *reply) override 
    {
        std::fstream ifile("addr", std::ios::binary | std::ios::in);
        tutorial::AddressBook addr_book;
        if (not addr_book.ParseFromIstream(&ifile))
        {
            std::cerr << "Failed to parse address book \"addr\"!" << std::endl;
            return Status::CANCELLED;
        }
        std::string name = request->name();
        CreatePersonInfo(addr_book, name, reply);

        return Status::OK;
    }
};

void RunServer()
{
    std::string server_address("0.0.0.0:50051");
    GetInfoServiceImpl service;
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
