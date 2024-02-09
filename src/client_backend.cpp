#include "client_backend.hpp"

#include <QApplication>
#include <QMetaType>
#include <algorithm>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <zmq.hpp>

#include "chat_client_app.hpp"
#include "client_frontend.hpp"
#include "messages.pb.h"
#include "utils.hpp"

namespace chat_app {
ClientBackend::ClientBackend(
    const std::string& name, const std::string& push_addr, const std::string& sub_addr, const std::string& req_addr,
    std::queue<std::shared_ptr<std::tuple<const std::string, const std::string>>>* received_messages_queue,
    std::queue<std::shared_ptr<std::vector<std::string>>>* active_users_queue)
    : push_socket_{ctx_, zmq::socket_type::push},
      sub_socket_{ctx_, zmq::socket_type::sub},
      req_socket_{ctx_, zmq::socket_type::req},
      name_{name},
      stop_{false},
      stopped_{false},
      incoming_msgs_queue_{received_messages_queue},
      active_users_queue_{active_users_queue} {
  push_socket_.connect(push_addr);
  sub_socket_.set(zmq::sockopt::subscribe, "/all/");
  sub_socket_.set(zmq::sockopt::subscribe, ("/" + name_ + "/").c_str());
  sub_socket_.connect(sub_addr);
  req_socket_.set(zmq::sockopt::rcvtimeo, 1000);
  req_socket_.connect(req_addr);
  msg_subscription_thread_ = std::thread{&ClientBackend::ReceiveMessages, this};
  heartbeat_thread_ = std::thread{&ClientBackend::UpdateClientStatus, this};
}

void ClientBackend::Stop() {
  std::cout << "Entering Stop\n";
  stop_ = true;
  msg_subscription_thread_.join();
  heartbeat_thread_.join();
  stopped_ = true;
  std::cout << "Stopped ClientBackend - all looping threads have terminated\n";
}

bool ClientBackend::IsStopped() const { return stopped_; }

// Sends the given text message to the given group of recipients.
void ClientBackend::SendTextMessage(const std::string& text, const std::vector<std::string>& recipients) {
  auto msg = DataMsg();
  msg.set_from(name_);
  for (const auto& recipient : recipients) {
    msg.add_to(recipient);
  }
  msg.set_message_text(text);

  const auto result = utils::SendProtobufMessage(push_socket_, msg);
  std::cout << "Send result: " << result.value() << "\n";
}

// Receives incoming text messages via the sub_socket_.
void ClientBackend::ReceiveMessages() {
  std::cout << "ClientBackend: enter ReceiveMessages...\n";
  while (!stop_) {
    zmq::message_t topic;
    const auto recv_result_1 = sub_socket_.recv(topic, zmq::recv_flags::dontwait);
    if (!recv_result_1.has_value()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      continue;
    }
    int64_t more = sub_socket_.get(zmq::sockopt::rcvmore);
    if (more != 1) {
      std::cout << "Error receiving mesage - unexpected number of remaining message parts. Expected 1, got " << more
                << "\n";
      continue;
    }
    DataMsg rec_msg;
    const auto recv_result_2 = utils::ReceiveProtobufMessage(sub_socket_, rec_msg, zmq::recv_flags::dontwait);
    if (!recv_result_2.has_value()) {
      std::cout << "Error - expected to receive something after the topic!\n";
      continue;
    }
    if (rec_msg.from() == name_) {
      // Drop the own message (that's coming from this client)
      // This could e.g. be used to mark the message as "delivered"
      continue;
    }
    std::cout << "[" << name_ << "]: Received message \"" << rec_msg.message_text() << "\" on topic "
              << topic.to_string() << " from " << rec_msg.from() << "\n";
    incoming_msgs_queue_->push(std::make_shared<std::tuple<const std::string, const std::string>>(
        std::make_tuple(rec_msg.from(), rec_msg.message_text())));
  }
  std::cout << "ClientBackend: leave ReceiveMessages...\n";
}

// Sends the client heartbeat message to the server to signal that it's online.
// Returns the value of the req_socket_.send(heartbeat_msg) operation.
// Must be called before ReceiveActiveUsers()
zmq::send_result_t ClientBackend::SendHeartbeat() {
  HeartbeatMsg msg{};
  msg.set_from(name_);
  return utils::SendProtobufMessage(req_socket_, msg);
}

// Receives (in a response to the heartbeat sent) a list of active users
// and signals them to the Frontend via the active_users_queue_.
// Must be called after SendHeartbeat() only if it returned a successful send result.
zmq::recv_result_t ClientBackend::ReceiveActiveUsers() {
  HeartbeatMsg rec_msg;
  const auto recv_result = utils::ReceiveProtobufMessage(req_socket_, rec_msg);
  if (recv_result.has_value()) {
    for (const auto& user : rec_msg.active_users()) {
      std::cout << user << ", ";
    }
    std::cout << "\n";
    auto active_users = std::make_shared<std::vector<std::string>>();
    for (const auto& user : rec_msg.active_users()) {
      active_users->push_back(user);
    }
    active_users_queue_->push(active_users);
  } else {
    std::cout << "Warning: No response to heartbeat received!\n";
  }
  return recv_result;
}

// Calls SendHeartbeat() and ReceiveActiveUsers() functions one after another in a loop.
// Signals with an error message to stdout when no repsonse to the heartbeat is received from the Server.
// Runs in a separate thread.
// TODO there's still a bug that causes the entire client app to terminate when the Server stops.
void ClientBackend::UpdateClientStatus() {
  std::cout << "ClientBackend: enter UpdateClientStatus...\n";
  while (!stop_) {
    SendHeartbeat();
    const auto recv_result = ReceiveActiveUsers();
    if (!recv_result.has_value()) {
      std::cout << "Error: Couldn't connect to the server (didn't receive a list of active users). Please make sure "
                   "it's up and running and restart this client!\n";
      termination_thread_ =
          std::thread{&ClientBackend::Stop, this};  // TODO - would need to find a cleaner way for the stop
      break;                                        // Break to leave this thread
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  std::cout << "ClientBackend: leave UpdateClientStatus...\n";
  stopped_ = true;
}
}  // namespace chat_app