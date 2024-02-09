#include "chat_client_controller.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace chat_app {

ChatClientController::ChatClientController(const std::string& name, const bool auto_connect,
                                           const std::string& push_addr, const std::string& sub_addr,
                                           const std::string& req_addr)
    : user_name_{name},
      push_addr_{push_addr},
      sub_addr_{sub_addr},
      req_addr_{req_addr},
      outgoing_messages_queue_{},
      incoming_messages_queue_{},
      active_users_queue_{},
      connect_actions_queue_{},
      client_frontend_{&outgoing_messages_queue_, &connect_actions_queue_},
      client_backend_{nullptr},
      outgoing_msg_thread_{&ChatClientController::HandleOutgoingMessages, this},
      incoming_msg_thread_{&ChatClientController::HandleIncomingMessages, this},
      active_users_thread_{&ChatClientController::HandleActiveUsers, this},
      connect_actions_thread_{&ChatClientController::HandleConnectButton, this},
      connection_monitor_thread_{&ChatClientController::MonitorConnection, this}

{
  client_frontend_.setWindowTitle(QString::fromStdString("Chat-Service [" + name + "]"));
  client_frontend_.resize(600, 400);
  client_frontend_.show();

  if (auto_connect) {
    client_backend_ = std::make_unique<ClientBackend>(user_name_, push_addr_, sub_addr_, req_addr_,
                                                      &incoming_messages_queue_, &active_users_queue_);
  }
}

// Function that monitors the outgoing_messages_queue_ in a separate thread.
// It is filled by the Frontend and signals that the user wants to send a (text) message.
// When a new message arrives, it is sent via Backend's SendTextMessage function.
void ChatClientController::HandleOutgoingMessages() {
  std::cout << "ChatClientApp: HandleOutgoingMessages thread started\n";
  while (true) {
    if (!outgoing_messages_queue_.empty()) {
      const auto user_input = outgoing_messages_queue_.front();
      const auto msg_text = std::get<0>(user_input);
      const auto msg_recipients = std::get<1>(user_input);
      client_backend_->SendTextMessage(msg_text, msg_recipients);
      outgoing_messages_queue_.pop();
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

// Function that monitors the current status of the Backend and updates the Frontend status accordingly.
// Runs in a separate thread.
void ChatClientController::MonitorConnection() {
  std::cout << "ChatClientApp: MonitorConnection thread started\n";
  while (true) {
    if (!client_backend_ || client_backend_->IsStopped()) {
      if (client_backend_) {
        client_backend_.reset();
      }
      client_frontend_.SetConnectedStatus(false);
    } else {
      client_frontend_.SetConnectedStatus(true);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

// Function that reacts to events in the connect_actions_queue_ triggered by the user
// who pressed the (Dis)Connect button in Frontend.
// Runs in a separate thread.
void ChatClientController::HandleConnectButton() {
  std::cout << "ChatClientApp: HandleConnectButton thread started\n";
  while (true) {
    if (!connect_actions_queue_.empty()) {
      const auto connect_action = connect_actions_queue_.front();
      const bool connected = client_backend_ && !client_backend_->IsStopped();
      if (connect_action == UserConnectAction::kConnect && !connected) {
        if (!client_backend_) {
          client_backend_ = std::make_unique<ClientBackend>(user_name_, push_addr_, sub_addr_, req_addr_,
                                                            &incoming_messages_queue_, &active_users_queue_);
        }
      } else if (connect_action == UserConnectAction::kDisconnect && connected) {
        client_backend_->Stop();
        client_backend_.reset();
      }
      connect_actions_queue_.pop();
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

// Function that monitors the incoming_messages_queue_ in a separate thread.
// It is filled by the Backend and signals that a text message was received.
// The text message is then forwarded to the Frontend via its DisplayNewMessage function.
void ChatClientController::HandleIncomingMessages() {
  std::cout << "ChatClientApp: HandleIncomingMessages thread started\n";
  while (true) {
    if (!incoming_messages_queue_.empty()) {
      const auto tuple = incoming_messages_queue_.front();
      const auto msg_author = std::get<0>(*tuple);
      const auto msg_text = std::get<1>(*tuple);
      client_frontend_.DisplayNewMessage(msg_author, msg_text);
      incoming_messages_queue_.pop();
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

// Function that monitors the incoming_messages_queue_ in a separate thread.
// It is filled by the Backend and signals that a fresh list of active users was received.
// The list is then forwarded to the Frontend via its UpdateActiveUsersList function.
void ChatClientController::HandleActiveUsers() {
  std::cout << "ChatClientApp: HandleActiveUsers thread started\n";
  while (true) {
    if (!active_users_queue_.empty()) {
      client_frontend_.UpdateActiveUsersList(active_users_queue_.front());
      active_users_queue_.pop();
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}
}  // namespace chat_app