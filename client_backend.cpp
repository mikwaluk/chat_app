#include <zmq.hpp>
#include <string>
#include <thread>
#include <iostream>

#include <vector>

#include "test.pb.h"

#include "client_backend.hpp"
#include <algorithm>
#include "client_frontend.hpp"
#include "chat_client_app.hpp"

#include <QApplication>
#include <QMetaType>

namespace chat_app {
  ClientBackend::ClientBackend(const std::string& name, const std::string& push_addr, const std::string& sub_addr, const std::string& req_addr, std::queue<std::tuple<const std::string, const std::string>>* received_messages_queue, std::queue<std::shared_ptr<std::vector<std::string>>>* active_users_queue)
    : push_socket_ {ctx_, zmq::socket_type::push}
    , sub_socket_ {ctx_, zmq::socket_type::sub}
    , req_socket_ {ctx_, zmq::socket_type::req}
    , name_ {name}
    , incoming_msgs_queue_ {received_messages_queue}
    , active_users_queue_ {active_users_queue}
  {
    push_socket_.connect(push_addr);
    sub_socket_.set(zmq::sockopt::subscribe, "/all/");
    sub_socket_.set(zmq::sockopt::subscribe, ("/" + name_ + "/").c_str());
    sub_socket_.connect(sub_addr);
    req_socket_.set(zmq::sockopt::rcvtimeo, 1000);
    req_socket_.connect(req_addr);
    msg_subscription_thread_ = std::thread {&ClientBackend::ReceiveMessages, this};
    heartbeat_thread_ = std::thread {&ClientBackend::UpdateClientStatus, this};
  }

  zmq::send_result_t ClientBackend::SendProtobufMessage(zmq::socket_t& socket, const google::protobuf::Message& msg, zmq::send_flags flags) {
    std::string serialized_output;
    msg.SerializeToString(&serialized_output);
    zmq::message_t zmq_msg {serialized_output.size()};
    memcpy((void *)zmq_msg.data(), serialized_output.c_str(), serialized_output.size());
    const auto result = socket.send(zmq_msg, flags);
    return result;
  }

  void ClientBackend::SendTextMessage(const std::string& text, const std::vector<std::string>& recipients) {
    auto msg = test::TestMsg();
    msg.set_from(name_);
    for (const auto& recipient : recipients) {
      msg.add_to(recipient);
    }
    msg.set_message_text(text);

    const auto result = SendProtobufMessage(push_socket_, msg);
    std::cout << "Send result: " << result.value() << "\n";
  }

  void ClientBackend::ReceiveMessages() {
    std::cout << "Start receiving...\n";
    while (true) {
      zmq::message_t topic;
      const auto recv_result_1 = sub_socket_.recv(topic, zmq::recv_flags::dontwait);
      if (!recv_result_1.has_value()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }
      int64_t more = sub_socket_.get(zmq::sockopt::rcvmore);
      if (more != 1) {
        std::cout << "Error receiving mesage - unexpected number of remaining message parts. Expected 1, got " << more << "\n";
        continue;
      }
      zmq::message_t rec_data;
      const auto recv_result_2 = sub_socket_.recv(rec_data, zmq::recv_flags::dontwait);
      if (!recv_result_2.has_value()) {
        std::cout << "Error - expected to receive something after the topic!\n";
        continue;
      }
      test::TestMsg rec_msg;
      rec_msg.ParseFromArray(rec_data.data(), rec_data.size());
      if (rec_msg.from() == name_) {
        // Drop your own message
        // This could e.g. be later used to mark the message as "delivered"
        // TODO adjust the server? or just ask Joerg Engel!
        continue;
      }
      std::cout << "[" << name_ << "]: Received message \"" << rec_msg.message_text() << "\" on topic " << topic.to_string() << " from " << rec_msg.from() << "\n";
      incoming_msgs_queue_->push(std::make_tuple(rec_msg.from(), rec_msg.message_text()));
    }
  }

  zmq::send_result_t ClientBackend::SendHeartbeat() {
    test::HeartbeatMsg msg {};
    msg.set_from(name_);
    return SendProtobufMessage(req_socket_, msg);
  }

  zmq::recv_result_t ClientBackend::ReceiveActiveUsers() {
      zmq::message_t rec_data;
      const auto recv_result = req_socket_.recv(rec_data);
      if (recv_result.has_value()) {
        test::HeartbeatMsg rec_msg;
        rec_msg.ParseFromArray(rec_data.data(), rec_data.size());

        for (const auto& user : rec_msg.active_users()) {
          std::cout << user << "\n"; // TODO use some logging library
        }
        auto active_users = std::make_shared<std::vector<std::string>>();
        for (const auto& user : rec_msg.active_users()) {
          active_users->push_back(user);
        }
        active_users_queue_->push(active_users);
      }
      else {
        std::cout << "No heartbeat received!\n";
      }
      return recv_result;
  }

  void ClientBackend::UpdateClientStatus() {
    while (true) {
      SendHeartbeat();
      const auto recv_result = ReceiveActiveUsers();
      if (!recv_result.has_value()) {
        std::cout << "Error: Couldn't connect to the server (didn't receive a list of active users). Please make sure it's up and running!\n";
        break;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
} // namespace chat_app