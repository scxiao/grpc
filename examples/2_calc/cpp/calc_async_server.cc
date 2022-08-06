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

    class AddCallData : public CallData
    {
    public:
        AddCallData(Calculator::AsyncService* service, ServerCompletionQueue* queue)
            : CallData{service, queue}, responder_{&context_}
        {
            tag_.id = MessageID::ADD;
            tag_.call = this;
            Proceed();
        }

        void Proceed() override
        {
            switch(status_)
            {
            case CallStatus::CREATE:
                status_ = CallStatus::PROCESS;
                service_->RequestAdd(&context_, &request_, &responder_, queue_, queue_, (void*)&tag_);
                break;

            case CallStatus::PROCESS:
                new AddCallData{service_, queue_};
                response_.set_sum(request_.addend_1() + request_.addend_2());
                status_ = CallStatus::FINISH;
                responder_.Finish(response_, Status::OK, (void*)&tag_);
                break;
            default:
                delete this;
            }
        }

    private:
        AddRequest request_;
        AddResponse response_;
        ServerAsyncResponseWriter<AddResponse> responder_;
        Tag tag_;
    };

    class SubCallData : public CallData {
    public:
        SubCallData(Calculator::AsyncService* service, ServerCompletionQueue* queue)
                : CallData{service, queue}, _responder{&context_} {
            _tag.id = MessageID::SUB;
            _tag.call = this;
            Proceed();
        }

        void Proceed() override {
            switch (status_) {
                case CallStatus::CREATE: {
                    status_ = CallStatus::PROCESS;
                    service_->RequestSub(&context_, &_request, &_responder, queue_, queue_, (void*)&_tag);
                    break;
                }
                case CallStatus::PROCESS: {
                    new SubCallData{service_, queue_};
                    _response.set_difference(_request.minuend() - _request.subtrahend());
                    status_ = CallStatus::FINISH;
                    _responder.Finish(_response, Status::OK, (void*)&_tag);
                    break;
                }
                default: {
                    delete this;
                }
            }
        }

    private:
        SubRequest _request;
        SubResponse _response;
        ServerAsyncResponseWriter<SubResponse> _responder;
        Tag _tag;
    };

    class MulCallData : public CallData {
    public:
        MulCallData(Calculator::AsyncService* service, ServerCompletionQueue* queue)
                : CallData{service, queue}, _responder{&context_} {
            _tag.id = MessageID::MUL;
            _tag.call = this;
            Proceed();
        }

        void Proceed() override {
            switch (status_) {
                case CallStatus::CREATE: {
                    status_ = CallStatus::PROCESS;
                    service_->RequestMul(&context_, &_request, &_responder, queue_, queue_, (void*)&_tag);
                    break;
                }
                case CallStatus::PROCESS: {
                    new MulCallData{service_, queue_};
                    _response.set_product(_request.multiplicand() * _request.multiplier());
                    status_ = CallStatus::FINISH;
                    _responder.Finish(_response, Status::OK, (void*)&_tag);
                    break;
                }
                default: {
                    delete this;
                }
            }
        }

    private:
        MulRequest _request;
        MulResponse _response;
        ServerAsyncResponseWriter<MulResponse> _responder;
        Tag _tag;
    };

    class DivCallData : public CallData {
    public:
        DivCallData(Calculator::AsyncService* service, ServerCompletionQueue* queue)
                : CallData{service, queue}, _responder{&context_} {
            _tag.id = MessageID::DIV;
            _tag.call = this;
            Proceed();
        }

        void Proceed() override {
            switch (status_) {
                case CallStatus::CREATE: {
                    status_ = CallStatus::PROCESS;
                    service_->RequestDiv(&context_, &_request, &_responder, queue_, queue_, (void*)&_tag);
                    break;
                }
                case CallStatus::PROCESS: {
                    new DivCallData{service_, queue_};
                    _response.set_quotient(_request.dividend() / _request.divisor());
                    status_ = CallStatus::FINISH;
                    _responder.Finish(_response, Status::OK, (void*)&_tag);
                    break;
                }
                default: {
                    delete this;
                }
            }
        }

    private:
        DivRequest _request;
        DivResponse _response;
        ServerAsyncResponseWriter<DivResponse> _responder;
        Tag _tag;
    };

    class ModCallData : public CallData {
    public:
        ModCallData(Calculator::AsyncService* service, ServerCompletionQueue* queue)
                : CallData{service, queue}, _responder{&context_} {
            _tag.id = MessageID::MOD;
            _tag.call = this;
            Proceed();
        }

        void Proceed() override {
            switch (status_) {
                case CallStatus::CREATE: {
                    status_ = CallStatus::PROCESS;
                    service_->RequestMod(&context_, &_request, &_responder, queue_, queue_, (void*)&_tag);
                    break;
                }
                case CallStatus::PROCESS: {
                    new ModCallData{service_, queue_};
                    _response.set_result(_request.dividend() % _request.divisor());
                    status_ = CallStatus::FINISH;
                    _responder.Finish(_response, Status::OK, (void*)&_tag);
                    break;
                }
                default: {
                    delete this;
                }
            }
        }

    private:
        ModRequest _request;
        ModResponse _response;
        ServerAsyncResponseWriter<ModResponse> _responder;
        Tag _tag;
    };

    void HandleRPCs()
    {
        new AddCallData{&service_, queue_.get()};
        new SubCallData{&service_, queue_.get()};
        new MulCallData{&service_, queue_.get()};
        new DivCallData{&service_, queue_.get()};
        new ModCallData{&service_, queue_.get()};

        void *tag;
        bool ok;
        while (true)
        {
            if (queue_->Next(&tag, &ok) and ok)
            {
                auto* tag_ptr = static_cast<Tag*>(tag);
                switch(tag_ptr->id)
                {
                case MessageID::ADD:
                    static_cast<AddCallData*>(tag_ptr->call)->Proceed();
                    break;
                case MessageID::SUB:
                    static_cast<SubCallData*>(tag_ptr->call)->Proceed();
                    break;
                case MessageID::MUL:
                    static_cast<MulCallData*>(tag_ptr->call)->Proceed();
                    break;
                case MessageID::DIV:
                    static_cast<DivCallData*>(tag_ptr->call)->Proceed();
                    break;
                case MessageID::MOD:
                    static_cast<ModCallData*>(tag_ptr->call)->Proceed();
                    break;
                }
            }
            else
            {
                std::cerr << "Something went wrong" << std::endl;
                std::abort();
            }
        }
    }


private:
    Calculator::AsyncService service_;
    std::unique_ptr<ServerCompletionQueue> queue_;
    std::unique_ptr<Server> server_;
};

int main(int argc, char **argv)
{
    CalcServer server;
    server.Run();

    return 0;
}
