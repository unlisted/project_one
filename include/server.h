#include <grpc/grpc.h>

#include <proto/node.pb.h>
#include <proto/node.grpc.pb.h>

using grpc::Status;
using grpc::ServerReader;
using grpc::ServerWriter;
using grpc::ServerContext;

class NodeServerImpl final : public NodeServer::Service {
public:
    Status GetValue(ServerContext* context, const Key* key, Value* value) override;
    Status SetValue(ServerContext* context, const KeyValue* kv, SetValueResponse* response) override;
    Status RequestKeys(ServerContext* context, const RequestKeysParams* params, ServerWriter<KeyValue>* writer) override;
    Status SendKeys(ServerContext* context, ServerReader<KeyValue>* reader, SendKeysResponse* response) override;
    
};

void RunServer();