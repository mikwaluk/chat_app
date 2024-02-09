#ifndef CLIENT_BACKEND_HPP_
#define CLIENT_BACKEND_HPP_

#include <queue>
#include <thread>
#include <zmq.hpp>

#include "test.pb.h"

namespace chat_app {
// Handles the ZMQ communication with the Server;
// Communicates with the Frontend via queues (incoming_msgs_queue_ and active_users_queue_).
// Uses 3 different sockets to communicate with the Server:
// - push_socket_ to send outgoing messages to the Server (ZMQ Push/Pull pattern)
// - sub_socket_ to receive incoming messages from the Server (ZMQ Pub/Sub pattern)
// - req_socket_ to send the hearbeat message and receive back a list of active users (ZMQ Req/Rep pattern)
class ClientBackend {
 private:
  zmq::context_t ctx_;
  zmq::socket_t push_socket_;
  zmq::socket_t sub_socket_;
  zmq::socket_t req_socket_;
  const std::string name_;
  std::queue<std::shared_ptr<std::tuple<const std::string, const std::string>>>* incoming_msgs_queue_;
  std::queue<std::shared_ptr<std::vector<std::string>>>* active_users_queue_;

 public:
  std::thread msg_subscription_thread_;
  std::thread heartbeat_thread_;
  ClientBackend(const std::string& name, const std::string& push_addr, const std::string& sub_addr,
                const std::string& req_addr,
                std::queue<std::shared_ptr<std::tuple<const std::string, const std::string>>>* received_messages_queue,
                std::queue<std::shared_ptr<std::vector<std::string>>>* active_users_queue);

  void SendTextMessage(const std::string& text, const std::vector<std::string>& recipients);
  void ReceiveMessages();

  void UpdateClientStatus();

  zmq::send_result_t SendHeartbeat();

  zmq::recv_result_t ReceiveActiveUsers();
};
}  // namespace chat_app
#endif  // CLIENT_BACKEND_HPP_