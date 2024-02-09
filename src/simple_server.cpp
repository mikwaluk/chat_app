#include <iostream>
#include <string>
#include <thread>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "test.pb.h"
#include "user_manager.hpp"

// TODO rewrite - taken from the Internet. Maybe use command line arguments instead of env vars?
std::string GetEnv(const std::string& var) {
  const char* val = std::getenv(var.c_str());
  if (val == nullptr) {  // invalid to assign nullptr to std::string
    return "";
  } else {
    return val;
  }
}
// TODO catch the signal in the main function - and destroy the object before exiting
class SimpleServer {
  zmq::context_t ctx_;
  zmq::socket_t pull_socket_;
  zmq::socket_t pub_socket_;
  zmq::socket_t resp_socket_;
  chat_app::UserManager user_manager_;

 public:
  void receiveMessage() {
    std::cout << "Start receiving...\n";
    while (true) {
      zmq::message_t t;
      pull_socket_.recv(t);
      test::TestMsg rec_data;
      rec_data.ParseFromArray(t.data(), t.size());
      std::cout << " Received \"" << rec_data.message_text() << "\" from " << rec_data.from() << " to ";
      if (rec_data.to().size() > 0) {
        for (const auto& recipient : rec_data.to()) {
          std::cout << recipient << ",";
          const auto recipient_namespace = "/" + recipient + "/";
          std::cout << "Recipient: " << recipient_namespace << "\n";
          auto ns_message = zmq::message_t((void*)recipient_namespace.c_str(), recipient_namespace.length());
          pub_socket_.send(ns_message, zmq::send_flags::sndmore);
        }
      } else {
        std::cout << "all";
        const auto recipient_namespace = std::string{"/all/"};
        std::cout << "Recipient: " << recipient_namespace << "\n";
        auto ns_message = zmq::message_t((void*)recipient_namespace.c_str(), recipient_namespace.length());
        pub_socket_.send(ns_message, zmq::send_flags::sndmore);
      }
      std::cout << "\n";
      pub_socket_.send(t, zmq::send_flags::none);
    }
  }

  void receiveHeartbeat() {
    std::cout << "Start waiting for heartbeats...\n";
    while (true) {
      zmq::message_t t;
      resp_socket_.recv(t);
      test::HeartbeatMsg rec_data;
      rec_data.ParseFromArray(t.data(), t.size());
      std::cout << " Received heartbeat from " << rec_data.from() << "\n";
      user_manager_.UpdateActiveUser(rec_data.from());
      for (const auto& user : user_manager_.GetActiveUsers()) {
        rec_data.add_active_users(user);
      }
      std::string serialized_output;
      rec_data.SerializeToString(&serialized_output);
      zmq::message_t response(serialized_output.size());
      memcpy((void*)response.data(), serialized_output.c_str(), serialized_output.size());
      resp_socket_.send(response, zmq::send_flags::none);
    }
  }

  SimpleServer()
      : pull_socket_{ctx_, zmq::socket_type::pull},
        pub_socket_{ctx_, zmq::socket_type::pub},
        resp_socket_{ctx_, zmq::socket_type::rep},
        user_manager_{} {
    const std::string server_host = GetEnv("ZMQ_SERVER_HOST_ADRESS");
    assert(server_host != "");
    const std::string server_pull_port = GetEnv("ZMQ_SERVER_PULL_PORT");
    assert(server_pull_port != "");
    const std::string server_pub_port = GetEnv("ZMQ_SERVER_PUB_PORT");
    assert(server_pub_port != "");
    const std::string server_resp_port{"8002"};
    assert(server_resp_port != "");
    // Can be made a separate function
    const std::string server_pull_socket_address =
        std::string("tcp://") + server_host + std::string(":") + server_pull_port;
    const std::string server_pub_socket_address =
        std::string("tcp://") + server_host + std::string(":") + server_pub_port;
    const std::string server_rep_socket_address =
        std::string("tcp://") + server_host + std::string(":") + server_resp_port;
    pull_socket_.bind(server_pull_socket_address);
    pub_socket_.bind(server_pub_socket_address);
    resp_socket_.bind(server_rep_socket_address);
    std::thread t1(&SimpleServer::receiveMessage, this);
    std::thread t2(&SimpleServer::receiveHeartbeat, this);
    t1.join();
  }
  ~SimpleServer() { std::cout << "\nDestructor executed"; }
};

int main() { SimpleServer s{}; }