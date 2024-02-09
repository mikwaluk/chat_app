#ifndef SIMPLE_CLIENT_HPP_
#define SIMPLE_CLIENT_HPP_

#include <zmq.hpp>
#include <thread>

#include "test.pb.h"

namespace chat_app {
    class SimpleClient {
    private:
    zmq::context_t ctx_;
    zmq::socket_t push_socket_;
    zmq::socket_t sub_socket_;
    const std::string name_;
    bool stop_;
    public:
    std::thread t1;
    SimpleClient(const std::string& name, const std::string& push_addr, const std::string& sub_addr);
    void SendTextMessage(const std::string& text, const std::vector<std::string>& recipients);
    void ReceiveMessage();
    void Stop();
    ~SimpleClient();
    };
}
#endif  // SIMPLE_CLIENT_HPP_