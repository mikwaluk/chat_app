#include "server.hpp"

#include <iostream>
#include <zmq.hpp>

#include "messages.pb.h"
#include "user_manager.hpp"
#include "utils.hpp"
namespace chat_app {

Server::Server(const std::string& pull_addr, const std::string& pub_addr, const std::string& rep_addr)
    : pull_socket_{ctx_, zmq::socket_type::pull},
      pub_socket_{ctx_, zmq::socket_type::pub},
      resp_socket_{ctx_, zmq::socket_type::rep},
      user_manager_{} {
  pull_socket_.bind(pull_addr);
  pub_socket_.bind(pub_addr);
  resp_socket_.bind(rep_addr);
  receive_messages_thread_ = std::thread{&Server::ReceiveMessages, this};
  receive_heartbeats_thread_ = std::thread{&Server::ReceiveHeartbeats, this};
}

// This function listens to (text) messages sent from the clients.
// It extract the recipients from the protobuf and depending whether there are recipients specified or not,
// it either publishes a single message to the "/all/" topic, or sends a list of separate messages
// to single recipients' topics.
// The topics are specified using the multipart send feature of ZMQ.
void Server::ReceiveMessages() {
  std::cout << "Start receiving...\n";
  while (true) {
    zmq::message_t t;
    pull_socket_.recv(t);
    DataMsg rec_data;
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

// This function listens to incoming heartbeats from active users and passes them to the user_manager_.
// Then, it responds to the heartbeart sender with an up-to-date list of active users.
// It runs within its own thread.
void Server::ReceiveHeartbeats() {
  std::cout << "Start waiting for heartbeats...\n";
  while (true) {
    HeartbeatMsg rec_data;
    utils::ReceiveProtobufMessage(resp_socket_, rec_data);
    std::cout << " Received heartbeat from " << rec_data.from() << "\n";
    user_manager_.UpdateActiveUser(rec_data.from());
    for (const auto& user : user_manager_.GetActiveUsers()) {
      rec_data.add_active_users(user);
    }
    utils::SendProtobufMessage(resp_socket_, rec_data);
  }
}

// A dummy function that should just run indefinitely, until the program is terminated.
void Server::Serve() { receive_messages_thread_.join(); }
}  // namespace chat_app