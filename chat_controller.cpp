#include "chat_controller.hpp"
#include <string>
#include <vector>
#include <thread>
#include <iostream>
namespace chat_app {

      ChatController::ChatController(const std::string& name, const std::string& push_addr, const std::string& sub_addr)
      : simple_client_ {name, push_addr, sub_addr}
      , chat_window_ {&sent_messages_queue_}
      , sent_messages_queue_ {}
      , stop_ {false}
      , outgoing_msg_processor_ {&ChatController::HandleUserActions, this}
      {
        chat_window_.setWindowTitle("Group Chat");
        chat_window_.resize(600, 400);
        chat_window_.show();
        std::cout << "Initialized ChatController!\n";
      }

      void ChatController::HandleUserActions() {
        while (!stop_) {
            if (!sent_messages_queue_.empty()) {
                const auto user_input = sent_messages_queue_.front();
                const auto msg_text = std::get<0>(user_input);
                const auto msg_recipients = std::get<1>(user_input);
                simple_client_.SendTextMessage(msg_text, msg_recipients);
                sent_messages_queue_.pop();
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
      }

      void ChatController::Stop() {
        stop_ = true;
        simple_client_.Stop();
        outgoing_msg_processor_.join();
      }

} // namespace chat_app