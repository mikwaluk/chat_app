#ifndef CHAT_CLIENT_CONTROLLER_HPP_
#define CHAT_CLIENT_CONTROLLER_HPP_

#include <string>
#include <vector>

#include "client_backend.hpp"
#include "client_frontend.hpp"

namespace chat_app {

// Manages the client app, connecting ClientFrontend and ClientBackend with queues
// that are monitored by separate threads.
// Also, reacts when the User wants to connect/disconnect by starting or stopping the Backend
// and updates the Frontend accordingly.
class ChatClientController {
 public:
  ChatClientController(const std::string& name, const bool auto_connect, const std::string& push_addr,
                       const std::string& sub_addr, const std::string& req_addr);

 private:
  void HandleOutgoingMessages();
  void HandleIncomingMessages();
  void HandleConnectButton();
  void HandleActiveUsers();
  void MonitorConnection();

  const std::string user_name_;
  const std::string push_addr_;
  const std::string sub_addr_;
  const std::string req_addr_;
  std::queue<std::tuple<const std::string, const std::vector<std::string>>> outgoing_messages_queue_;
  std::queue<std::shared_ptr<std::tuple<const std::string, const std::string>>> incoming_messages_queue_;
  std::queue<std::shared_ptr<std::vector<std::string>>> active_users_queue_;
  std::queue<UserConnectAction> connect_actions_queue_;

  ClientFrontend client_frontend_;
  std::unique_ptr<ClientBackend> client_backend_;

  std::thread outgoing_msg_thread_;
  std::thread incoming_msg_thread_;
  std::thread active_users_thread_;
  std::thread connect_actions_thread_;
  std::thread connection_monitor_thread_;
};

}  // namespace chat_app

#endif  // CHAT_CLIENT_APP_HPP_