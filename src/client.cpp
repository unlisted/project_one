#include <memory>
#include <iostream>
#include <ctime>
#include <vector>
#include <string>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

#include "proto/node.grpc.pb.h"
#include "proto/node.grpc.pb.h"

#include <client.h>

using namespace std;
using grpc::Channel;
using grpc::ClientContext;
using grpc::CreateChannel;
using grpc::ServerReader;
using grpc::ClientReader;


string GetOneValue(const std::string& key_name) {
    auto channel = CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    auto stub = NodeServer::NewStub(channel);

    ClientContext context;
    Value value;
    Key key;
    key.set_name(key_name);

    auto status = stub->GetValue(&context, key, &value);
    return value.data();
}

bool SetOneValue(const std::string& key, const std::string& data) {
    auto channel = CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    auto stub = NodeServer::NewStub(channel);

    ClientContext context;
    KeyValue kv;
    SetValueResponse response;
    
    auto key_name = kv.mutable_key();
    key_name->set_name(key);

    auto value = kv.mutable_value();
    value->set_version(0);
    value->set_created(time(nullptr));
    value->set_data(data);

    auto status = stub->SetValue(&context, kv, &response);

    return true;
}

void RequestKeys() {
    auto channel = CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    auto stub = NodeServer::NewStub(channel);

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

void SyncTest() {
    auto channel = CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    auto stub = NodeServer::NewStub(channel);

    ClientContext context;
    SendKeysResponse response;
    auto writer = stub->SendKeys(&context, &response);

    vector<string> keys = {"foo", "bar", "hello", "world"};
    for (auto &k : keys){
        KeyValue kv;
        
        auto key = kv.mutable_key();
        key->set_name(k);

        auto value = kv.mutable_value();
        value->set_version(0);
        value->set_created(time(nullptr));
        value->set_data(k);

        cout << "sending " << k << endl;
        writer->Write(kv);
    }

    writer->WritesDone();
    auto status = writer->Finish();

    cout << "received: " << response.received() << endl;
    cout << "updated: " << response.updated() << endl;

}