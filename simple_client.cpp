#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <string>
#include <thread>
#include <iostream>
#include <uuid/uuid.h>

#include "test.pb.h"

std::string GetEnv( const std::string & var ) {
     const char * val = std::getenv( var.c_str() );
     if ( val == nullptr ) { // invalid to assign nullptr to std::string
         return "";
     }
     else {
         return val;
     }
}

#include <vector>
int main() {
    zmq::context_t ctx;
    zmq::socket_t push_socket {ctx, zmq::socket_type::push};
    const std::string server_host = GetEnv("ZMQ_SERVER_HOST_ADRESS");
    assert(server_host != "");
    const std::string server_pull_port = GetEnv("ZMQ_SERVER_PULL_PORT");
    assert(server_pull_port != "");
    std::string server_pull_socket_address = std::string("tcp://") + server_host + std::string(":") + server_pull_port;
    push_socket.connect(server_pull_socket_address);
    auto msg = test::TestMsg();
    msg.set_from("mik");
    msg.add_to("kasiunia");
    msg.set_name("koko");


    auto v = std::vector<test::UUID>();
    

    for (int i = 0; i < 10; i++) {
        srand(time(NULL));
        uuid_t uuid_c_struct;
        uuid_generate_time(uuid_c_struct);
        char uuid_str[37];
        uuid_unparse(uuid_c_struct, uuid_str);
        auto uuid_proto = test::UUID();
        uuid_proto.set_value(uuid_str);
        v.push_back(uuid_proto);
    }
    for (const auto& elem : v) {
        // Convert the uuid_t to a string representation




        msg.mutable_uuid()->CopyFrom(elem);
        std::string output;
        msg.SerializeToString(&output);
        zmq::message_t request (output.size());
        memcpy ((void *) request.data (), output.c_str(), output.size());
        push_socket.send(request);
    }
}