#include "chat_client_app.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace chat_app {

ChatClientApp::ChatClientApp(const std::string& name, const std::string& push_addr, const std::string& sub_addr,
                             const std::string& req_addr)
    : outgoing_messages_queue_{},
      incoming_messages_queue_{},
      active_users_queue_{},
      client_frontend_{&outgoing_messages_queue_},
      client_backend_{name, push_addr, sub_addr, req_addr, &incoming_messages_queue_, &active_users_queue_},
      outgoing_msg_thread_{&ChatClientApp::HandleOutgoingMessages, this},
      incoming_msg_thread_{&ChatClientApp::HandleIncomingMessages, this},
      active_users_thread_{&ChatClientApp::HandleActiveUsers, this}

{
  client_frontend_.setWindowTitle(QString::fromStdString("Chat-Service [" + name + "]"));
  client_frontend_.resize(600, 400);
  client_frontend_.show();
}

// Function that monitors the outgoing_messages_queue_ in a separate thread.
// It is filled by the Frontend and signals that the user wants to send a (text) message.
// When a new message arrives, it is sent via Backend's SendTextMessage function.
void ChatClientApp::HandleOutgoingMessages() {
  std::cout << "HandleOutgoingMessages thread started\n";
  while (true) {
    if (!outgoing_messages_queue_.empty()) {
      const auto user_input = outgoing_messages_queue_.front();
      const auto msg_text = std::get<0>(user_input);
      const auto msg_recipients = std::get<1>(user_input);
      client_backend_.SendTextMessage(msg_text, msg_recipients);
      outgoing_messages_queue_.pop();
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

// Function that monitors the incoming_messages_queue_ in a separate thread.
// It is filled by the Backend and signals that a text message was received.
// The text message is then forwarded to the Frontend via its DisplayNewMessage function.
void ChatClientApp::HandleIncomingMessages() {
  std::cout << "HandleIncomingMessages thread started\n";
  while (true) {
    if (!incoming_messages_queue_.empty()) {
      const auto tuple = incoming_messages_queue_.front();
      const auto msg_author = std::get<0>(tuple);
      const auto msg_text = std::get<1>(tuple);
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
void ChatClientApp::HandleActiveUsers() {
  std::cout << "HandleActiveUsers thread started\n";
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