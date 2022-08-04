#include <calc.grpc.pb.h>
#include <grpc++/grpc++.h>
#include <memory>
#include <iostream>

using namespace grpc;
using namespace calculator;

class CalculatorServerImpl : public Calculator::Service 
{
    Status Add(ServerContext* ctx, const AddRequest* req, AddResponse* response)
    {
        response->set_sum(req->addend_1() + req->addend_2());
        return Status::OK;
    }

    Status Sub(ServerContext* context, const SubRequest* request, SubResponse* response) {
        response->set_difference(request->minuend() - request->subtrahend());
        return Status::OK;
    }

    Status Mul(ServerContext* context, const MulRequest* request, MulResponse* response) {
        response->set_product(request->multiplicand() * request->multiplier());
        return Status::OK;
    }

    Status Div(ServerContext* context, const DivRequest* request, DivResponse* response) {
        response->set_quotient(request->dividend() / request->divisor());
        return Status::OK;
    }

    Status Mod(ServerContext* context, const ModRequest* request, ModResponse* response) {
        response->set_result(request->dividend() % request->divisor());
        return Status::OK;
    }
};

void RunServer()
{
    std::string server_address("localhost:2510");
    CalculatorServerImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server{builder.BuildAndStart()};

    // Run server
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main()
{
    RunServer();

    return 0;
}
