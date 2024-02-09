#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <string>
#include <zmq.hpp>

#include "user_manager.hpp"

namespace chat_app {

// A Group Chat Server which forwards (text) messages received from connected clients.
// It checks the recipient list of the message and forwards the message accordingly
// to proper ZMQ topics.
// It also monitors the active users by checking their heartbeats and maintains an up-to-date
// list of them, that is shared with the users in response to the heartbeat.
class Server {
 private:
  zmq::context_t ctx_;
  zmq::socket_t pull_socket_;
  zmq::socket_t pub_socket_;
  zmq::socket_t resp_socket_;
  UserManager user_manager_;
  std::thread receive_messages_thread_;
  std::thread receive_heartbeats_thread_;

 public:
  Server(const std::string& pull_addr, const std::string& pub_addr, const std::string& rep_addr);
  void ReceiveMessages();
  void ReceiveHeartbeats();
  void Serve();
};

}  // namespace chat_app

#endif  // SERVER_HPP_