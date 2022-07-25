#include <iostream>
#include <memory>
#include <string>

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

void CreatePersonInfo(Person *person_info)
{

}

class GetInfoServiceImpl final : public Greeter::Service
{
    Status GetInfo(ServerContext *context, const PersonRequest *request,
                   Person *reply) override 
    {
        std::string name = request->name();
        CreatePersonInfo(reply);

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
