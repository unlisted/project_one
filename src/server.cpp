#include <map>
#include <memory>
#include <string>
#include <ctime>
#include <thread>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

#include "proto/node.grpc.pb.h"
#include "proto/node.grpc.pb.h"


#include "server.h"

using namespace std;
using grpc::ServerBuilder;
using grpc::Server;
using grpc::ClientContext;
using grpc::CreateChannel;
using grpc::ClientReader;

map<string, Value> store;
mutex mtx;

bool add_new_entry(const string& key, const Value& val) {
    auto iter = store.find(key);
    if (iter == store.end()) {
        lock_guard<mutex> lck(mtx);
        store[key] = val;
        return true;
    } 
    // we could compare dates and versions, but for now just don't update
    // if key already exists
    return false;
}

Status NodeServerImpl::GetValue(ServerContext* context, const Key* key, Value* value) {
    auto iter = store.find(key->name());
    if (iter != store.end()) {
        value->CopyFrom(iter->second);
    }
    return Status::OK;
}

Status NodeServerImpl::SetValue(ServerContext* context, const KeyValue* kv, SetValueResponse* response) {
    auto key = kv->key().name();
    auto value = kv->value();

    auto added = add_new_entry(key, value);
    response->set_result(added ? SetValueResponse_Result_ADDED : SetValueResponse_Result_EXISTS);
    return Status::OK;
}

Status NodeServerImpl::SendKeys(ServerContext* context, ServerReader<KeyValue>* reader, SendKeysResponse* response) {
    KeyValue kv;
    int updated = 0;
    int received = 0;
    while(reader->Read(&kv)) {
        received++;
        cout << "received " << kv.key().name() << endl;
        if (add_new_entry(kv.key().name(), kv.value()))
            updated++;
    }
    response->set_received(received);
    response->set_updated(updated);
    return Status::OK;
}

Status NodeServerImpl::RequestKeys(ServerContext* context, const RequestKeysParams* params, ServerWriter<KeyValue>* writer) {

    auto limit = params->limit();
    size_t count = 0;
    for (auto& iter : store) {
        KeyValue kv;
        
        auto key = kv.mutable_key();
        key->set_name(iter.first);
        
        auto value = kv.mutable_value();
        value->CopyFrom(iter.second);

        writer->Write(kv);

        if (limit > 0 && ++count == limit) {
            cout << "limit " << limit << " reached, quitting." << endl;
        }
    }

    return Status::OK;
}

namespace clientserver {
class MyServer {
public:
    MyServer(const string& other, const string& listen = "0.0.0.0:50051") : _listen(listen) {
        node_list[other] = true;
        ServerBuilder builder;
        builder.AddListeningPort(_listen, grpc::InsecureServerCredentials());
        builder.RegisterService(&_service);
        _server = builder.BuildAndStart();

        Bootstrap();
        _server->Wait();
    }

    void Bootstrap() {
        auto channel = CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
        auto stub = ::NodeServer::NewStub(channel);

        ClientContext context;
        KeyValue kv;
        SetValueResponse response;

        RequestKeysParams params;
        params.set_limit(0);
        std::unique_ptr<ClientReader<KeyValue>>  reader(stub->RequestKeys(&context, params));
        while(reader->Read(&kv)) {
            cout << kv.key().name() << endl;
            cout << kv.value().data() << endl;
        }

        auto status = reader->Finish();    
    }
private:
    map<string, bool> node_list;
    string _listen;
    NodeServerImpl _service;
    std::unique_ptr<Server> _server;
};
}

void RunServer() {
    clientserver::MyServer ns("localhost:50051");
    
}

int main(int argc, char* argv[]) {
    thread t1(RunServer);
    t1.join();
    return 0;
}