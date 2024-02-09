#ifndef CHAT_CONTROLLER_HPP_
#define CHAT_CONTROLLER_HPP_


#include <string>
#include <vector>
#include "simple_client.hpp"
#include "client_frontend.hpp"

namespace chat_app {
 class ChatController {
  public:
      ChatController(const std::string& name, const std::string& push_addr, const std::string& sub_addr);
      void Stop();

  private:
      SimpleClient simple_client_;
      ChatWindow chat_window_;
      std::queue<std::tuple<const std::string, const std::vector<std::string>>> sent_messages_queue_;
      void HandleUserActions();
      bool stop_;
      std::thread outgoing_msg_processor_;
    };
} // namespace chat_app

#endif // CHAT_CONTROLLER_HPP_