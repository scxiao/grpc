#include <calc.grpc.pb.h>
#include <grpc++/grpc++.h>

#include <memory>
#include <thread>
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

class CalcClient
{
  public:
    CalcClient(std::shared_ptr<Channel> channel) : stub_(Calculator::NewStub(channel))
    { }

    void Add(int32_t addend_1, int32_t addend_2)
    {
        AddRequest req;
        req.set_addend_1(addend_1);
        req.set_addend_2(addend_2);

        auto* call = new AsyncClientCall<AddRequest, AddResponse>;
        call->request = req;
        call->rpc = stub_->PrepareAsyncAdd(&call->context, req, &queue_);
        call->rpc->StartCall();

        // Request to update the server's response and the call status
        // upon completion of the RPC
        auto* tag = new Tag;
        tag->call = (void*)call;
        tag->id = MessageID::ADD;
        call->rpc->Finish(&call->response, &call->status, (void*)tag);
    }

    void Sub(int32_t minuend, int32_t subtrahend)
    {
        SubRequest req;
        SubResponse response;
        req.set_minuend(minuend);
        req.set_subtrahend(subtrahend);

        auto *call = new AsyncClientCall<SubRequest, SubResponse>;
        call->request = req;
        call->rpc = stub_->PrepareAsyncSub(&call->context, req, &queue_);

        auto *tag = new Tag;
        tag->call = (void*)call;
        tag->id = MessageID::SUB;
        call->rpc->Finish(&call->response, &call->status, (void*)tag);
    }

    void Mul(int32_t multiplicand, int32_t multiplier) {
        // Prepare request
        MulRequest request;
        request.set_multiplicand(multiplicand);
        request.set_multiplier(multiplier);

        // Create an AsyncClientCall object to store RPC data
        auto* call = new AsyncClientCall<MulRequest, MulResponse>;
        call->request = request;

        // Create an RPC object
        call->rpc = stub_->PrepareAsyncMul(&call->context, request, &queue_);

        // Initiate the RPC call
        call->rpc->StartCall();

        // Request to update the server's response and the call status upon completion of the RPC
        auto* tag = new Tag;
        tag->call = (void*)call;
        tag->id = MessageID::MUL;
        call->rpc->Finish(&call->response, &call->status, (void*)tag);
    }

    void Div(int32_t dividend, int32_t divisor) {
        // Prepare request
        DivRequest request;
        request.set_dividend(dividend);
        request.set_divisor(divisor);

        // Create an AsyncClientCall object to store RPC data
        auto* call = new AsyncClientCall<DivRequest, DivResponse>;
        call->request = request;

        // Create an RPC object
        call->rpc = stub_->PrepareAsyncDiv(&call->context, request, &queue_);

        // Initiate the RPC call
        call->rpc->StartCall();

        // Request to update the server's response and the call status upon completion of the RPC
        auto* tag = new Tag;
        tag->call = (void*)call;
        tag->id = MessageID::DIV;
        call->rpc->Finish(&call->response, &call->status, (void*)tag);
    }

    void Mod(int32_t dividend, int32_t divisor) {
        // Prepare request
        ModRequest request;
        request.set_dividend(dividend);
        request.set_divisor(divisor);

        // Create an AsyncClientCall object to store RPC data
        auto* call = new AsyncClientCall<ModRequest, ModResponse>;
        call->request = request;

        // Create an RPC object
        call->rpc = stub_->PrepareAsyncMod(&call->context, request, &queue_);

        // Initiate the RPC call
        call->rpc->StartCall();

        // Request to update the server's response and the call status upon completion of the RPC
        auto* tag = new Tag;
        tag->call = (void*)call;
        tag->id = MessageID::MOD;
        call->rpc->Finish(&call->response, &call->status, (void*)tag);
    }

    void AsyncCompleteRPC()
    {
        void *tag;
        bool ok = false;
        while(queue_.Next(&tag, &ok))
        {
            auto *tag_ptr = static_cast<Tag*>(tag);
            if (not ok or not tag_ptr)
            {
                std::cerr << "Something went wrong" << std::endl;
                std::abort();
            }

            std::string err;
            switch(tag_ptr->id)
            {
                case MessageID::ADD: 
                    auto* call = static_cast<AsyncClientCall<AddRequest, AddResponse>*>(tag_ptr->call);
                    if (call)
                    {
                        if (call->status.ok())
                        {
                            std::cout << call->request.addend_1() << " + " << call->request.addend_2() << " = " << call->response.sum() << std::endl;
                        }
                        else
                        {
                            err = std::to_string(call->status.error_code()) + ": " + call->status.error_message();
                        }
                    }
                    else
                    {
                        err = "A client call was deleted";
                    }
                    delete call;
                    break;
                
                case MessageID::SUB: 
                    auto* call = static_cast<AsyncClientCall<SubRequest, SubResponse>*>(tag_ptr->call);
                    if (call) {
                        if (call->status.ok()) {
                            std::cout << call->request.minuend()
                                      << " - "
                                      << call->request.subtrahend()
                                      << " = "
                                      << call->response.difference() << std::endl;
                        } else {
                            err = std::to_string(call->status.error_code()) + ": " + call->status.error_message();
                        }
                    } else {
                        err = "A client call was deleted";
                    }
                    delete call;
                    break;
                
                case MessageID::MUL:
                    auto* call = static_cast<AsyncClientCall<MulRequest, MulResponse>*>(tag_ptr->call);
                    if (call) {
                        if (call->status.ok()) {
                            std::cout << call->request.multiplicand()
                                      << " * "
                                      << call->request.multiplier()
                                      << " = "
                                      << call->response.product() << std::endl;
                        } else {
                            err = std::to_string(call->status.error_code()) + ": " + call->status.error_message();
                        }
                    } else {
                        err = "A client call was deleted";
                    }
                    delete call;
                    break;
                
                case MessageID::DIV: 
                    auto* call = static_cast<AsyncClientCall<DivRequest, DivResponse>*>(tag_ptr->call);
                    if (call) {
                        if (call->status.ok()) {
                            std::cout << call->request.dividend()
                                      << " / "
                                      << call->request.divisor()
                                      << " = "
                                      << call->response.quotient() << std::endl;
                        } else {
                            err = std::to_string(call->status.error_code()) + ": " + call->status.error_message();
                        }
                    } else {
                        err = "A client call was deleted";
                    }
                    delete call;
                    break;
                
                case MessageID::MOD:
                    auto* call = static_cast<AsyncClientCall<ModRequest, ModResponse>*>(tag_ptr->call);
                    if (call) {
                        if (call->status.ok()) {
                            std::cout << call->request.dividend()
                                      << " % "
                                      << call->request.divisor()
                                      << " = "
                                      << call->response.result() << std::endl;
                        } else {
                            err = std::to_string(call->status.error_code()) + ": " + call->status.error_message();
                        }
                    } else {
                        err = "A client call was deleted";
                    }
                    delete call;
                    break;                    
            }

            delete tag_ptr;
            if (not err.empty())
            {
                throw std::runtime_error(err);
            }
        }
    }

  private:
    template<class RequestType, class ResponseType>
    struct AsyncClientCall
    {
        RequestType request;
        ResponseType response;
        ClientContext context;
        Status status;
        std::unique_ptr<ClientAsyncResponseReader<ResponseType>> rpc;
    };

    struct Tag
    {
        void *call;
        MessageID id;
    };

  private:
    std::unique_ptr<Calculator::Stub> stub_;
    CompletionQueue queue_;
};

void RunClient()
{
    std::string server_address("localhost:2511");
    CalcClient client{grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials())};
    std::thread thread(&CalcClient::AsyncCompleteRPC, &client);
    int rand = 4;
    for (int i = 0; i < 100; ++i)
    {
        switch(static_cast<MessageID>(i % 5))
        {
            case MessageID::ADD: {
                client.Add(i, rand);
                break;
            }
            case MessageID::SUB: {
                client.Sub(i, rand);
                break;
            }
            case MessageID::MUL: {
                client.Mul(i, rand);
                break;
            }
            case MessageID::DIV: {
                client.Div(i, rand);
                break;
            }
            case MessageID::MOD: {
                client.Mod(i, rand);
                break;
            }        
        }
    }

    std::cout << "Press Ctrl + C to quit ..." << std::endl << std::endl;
    thread.join();
}

int main(int argc, char **argv)
{
    RunClient();
    return 0;
}
