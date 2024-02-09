#include "chat_client_app.hpp"
#include <string>
#include <vector>
#include <thread>
#include <iostream>
namespace chat_app {

      ChatClientApp::ChatClientApp(const std::string& name, const std::string& push_addr, const std::string& sub_addr, const std::string& req_addr)
      : outgoing_messages_queue_ {}
      , incoming_messages_queue_ {}
      , active_users_queue_ {}
      , client_frontend_ {&outgoing_messages_queue_}
      , client_backend_ {name, push_addr, sub_addr, req_addr, &incoming_messages_queue_, &active_users_queue_}
      , stop_ {false}
      , outgoing_msg_handler_ {&ChatClientApp::HandleOutgoingMessages, this}
      , incoming_msg_handler_ {&ChatClientApp::HandleIncomingMessages, this}
      , active_users_handler_ {&ChatClientApp::HandleActiveUsers, this}

      {
        client_frontend_.setWindowTitle(QString::fromStdString("Chat-Service [" + name + "]"));
        client_frontend_.resize(600, 400);
        client_frontend_.show();
        std::cout << "Initialized ChatController!\n";
      }

      void ChatClientApp::HandleOutgoingMessages() {
        std::cout << "HandleOutgoingMessages thread started\n";
        while (!stop_) {
            if (!outgoing_messages_queue_.empty()) {
                const auto user_input = outgoing_messages_queue_.front();
                const auto msg_text = std::get<0>(user_input);
                const auto msg_recipients = std::get<1>(user_input);
                client_backend_.SendTextMessage(msg_text, msg_recipients);
                outgoing_messages_queue_.pop();
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
      }

      void ChatClientApp::HandleIncomingMessages() {
        std::cout << "HandleIncomingMessages thread started\n";
        while (!stop_) {
            if (!incoming_messages_queue_.empty()) {
                const auto tuple = incoming_messages_queue_.front();
                const auto msg_author = std::get<0>(tuple);
                const auto msg_text = std::get<1>(tuple);
                client_frontend_.DisplayNewMessage(msg_author, msg_text);
                incoming_messages_queue_.pop();
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
      }

    void ChatClientApp::HandleActiveUsers() {
        std::cout << "HandleActiveUsers thread started\n";
        while (!stop_) {
            if (!active_users_queue_.empty()) {
                std::cout << "active_users_queue_ not empty\n";
                client_frontend_.UpdateActiveUsersList(active_users_queue_.front());
                active_users_queue_.pop();
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
      }

      void ChatClientApp::Stop() {
        stop_ = true;
        outgoing_msg_handler_.join();
        incoming_msg_handler_.join();
      }

} // namespace chat_app