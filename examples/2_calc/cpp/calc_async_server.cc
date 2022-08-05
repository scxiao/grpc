#include <calc.grpc.pb.h>
#include <grpc++/grpc++.h>
#include <memory>
#include <iostream>

using namespace grpc;
using namespace calculator;

enum class MessageID : uint8_t 
{
    ADD = 0,
    SUB = 1,
    MUL = 2,
    DIV = 3,
    MOD = 4
};

class CalcServer
{
public:
    ~CalcServer()
    {
        server_->Shutdown();
        queue_->Shutdown();
    }

    void Run()
    {
        std::string server_address("localhost:2511");
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

        builder.RegisterService(&service_);
        queue_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();

        // Run server
        std::cout << "Server listening on " << server_address << std::endl;
        HandleRPCs();
    }

private:
    struct Tag
    {
        void *call;
        MessageID id;
    };

    class CallData
    {
    public:
        CallData(Calculator::AsyncService* service, ServerCompletionQueue* queue) :
            service_(service), queue_(queue), status_(CallStatus::CREATE)
        {

        }

        virtual void Proceed() = 0;

    protected:
        Calculator::AsyncService *service_;
        ServerCompletionQueue *queue_;
        ServerContext context_;
        enum class CallStatus
        {
            CREATE,
            PROCESS,
            FINISH
        };
        CallStatus status_;
    };

private:
    Calculator::AsyncService service_;
    std::unique_ptr<ServerCompletionQueue> queue_;
    std::unique_ptr<Server> server_;
};