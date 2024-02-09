#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <string>
#include <thread>
#include <iostream>

#include "test.pb.h"

// TODO rewrite - taken from the Internet. Maybe use command line arguments instead of env vars?
std::string GetEnv( const std::string & var ) {
     const char * val = std::getenv( var.c_str() );
     if ( val == nullptr ) { // invalid to assign nullptr to std::string
         return "";
     }
     else {
         return val;
     }
}
// TODO catch the signal in the main function - and destroy the object before exiting
class SimpleServer {
    zmq::context_t ctx;
    zmq::socket_t pull_socket;
    zmq::socket_t pub_socket;

    public:
    void receiveMessage(const std::string& msg)
    {
        std::cout << "Start receiving...\n";
        for (int i = 0; i < 10; i++) {
            zmq::message_t t;
            pull_socket.recv(t);
            std::cout << "somesing\n";
            test::TestMsg rec_data;
            rec_data.ParseFromArray(t.data(), t.size());
            std::cout << rec_data.uuid().value() << " Received \"" << rec_data.name() << "\" from " << rec_data.from() << " to ";
            if (rec_data.to().size() > 0) {
                for (const auto& recipient : rec_data.to()) {
                    std::cout << recipient << ",";
                }
            }
            else {
                std::cout << "all";
            }
            std::cout << "\n";
        }
    }
   //const std::string_view m = "Hello, world";
   //sock.send(zmq::buffer(m), zmq::send_flags::dontwait);
    SimpleServer() : pull_socket {ctx, zmq::socket_type::pull}, pub_socket {ctx, zmq::socket_type::pub} {
        const std::string server_host = GetEnv("ZMQ_SERVER_HOST_ADRESS");
        assert(server_host != "");
        const std::string server_pull_port = GetEnv("ZMQ_SERVER_PULL_PORT");
        assert(server_pull_port != "");
        const std::string server_pub_port = GetEnv("ZMQ_SERVER_PUB_PORT");
        assert(server_pub_port != "");
        // Can be made a separate function
        std::string server_pull_socket_address = std::string("tcp://") + server_host + std::string(":") + server_pull_port;
        std::string server_pub_socket_address = std::string("tcp://") + server_host + std::string(":") + server_pub_port;
        pull_socket.bind(server_pull_socket_address);
        pub_socket.bind(server_pub_socket_address);
        std::thread t1(&SimpleServer::receiveMessage, this, std::string("Test"));
        t1.join();
   }
   ~SimpleServer() { std::cout << "\nDestructor executed"; }
};

int main() {
    SimpleServer s {};
}