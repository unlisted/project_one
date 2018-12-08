// node has kv store
// accepts connections from clients
// talks to other nodes

#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <mutex>

#include <grpc/grpc.h>

#include <proto/node.pb.h>
#include <proto/node.grpc.pb.h>

#include "client.h"

using namespace std;

int main(int argc, char **argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // add_new_entry("foo", "bar");
    // //add_new_entry("foo", "world");
    
    // auto node = store.at("foo");
    // cout << node << endl;
    // auto entry_size = node->value_entry_size();
    // auto ssize = store.size();
    
    // cout << node->value_entry(0).value() << endl;;
    
    // fstream output("test.txt", ios::out | ios::trunc | ios::binary);
    // node.SerializeToOstream(&output);
    // output.close()

    SetOneValue("foo", "bar");
    cout << GetOneValue("foo") << endl;
    SyncTest();
    RequestKeys();


    return 0;
}