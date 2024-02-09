#ifndef CHAT_CLIENT_APP_HPP_
#define CHAT_CLIENT_APP_HPP_


#include <string>
#include <vector>
#include "client_backend.hpp"
#include "client_frontend.hpp"

#include "test.pb.h"

namespace chat_app {

  // Manages the client app, connecting ClientFrontend with ClientBackend using queues.
  class ChatClientApp {
    public:
      ChatClientApp(const std::string& name, const std::string& push_addr, const std::string& sub_addr, const std::string& req_addr);
      void Stop();

    private:
      std::queue<std::tuple<const std::string, const std::vector<std::string>>> outgoing_messages_queue_;
      std::queue<std::tuple<const std::string, const std::string>> incoming_messages_queue_;
      std::queue<std::shared_ptr<std::vector<std::string>>> active_users_queue_;

      ClientFrontend client_frontend_;
      ClientBackend client_backend_;

      void HandleOutgoingMessages();
      void HandleIncomingMessages();
      void HandleActiveUsers();
      bool stop_;
      std::thread outgoing_msg_handler_;
      std::thread incoming_msg_handler_;
      std::thread active_users_handler_;
    };
} // namespace chat_app

#endif // CHAT_CLIENT_APP_HPP_