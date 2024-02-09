#ifndef CLIENT_BACKEND_HPP_
#define CLIENT_BACKEND_HPP_

#include <zmq.hpp>
#include <thread>
#include <queue>

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
        std::queue<std::tuple<const std::string, const std::string>>* incoming_msgs_queue_;
        std::queue<std::shared_ptr<std::vector<std::string>>>* active_users_queue_;
    public:
    std::thread msg_subscription_thread_; // TODO do they even need to be members?
    std::thread heartbeat_thread_; // TODO do they even need to be members?
    ClientBackend(const std::string& name, const std::string& push_addr, const std::string& sub_addr, const std::string& req_addr, std::queue<std::tuple<const std::string, const std::string>>* received_messages_queue, std::queue<std::shared_ptr<std::vector<std::string>>>* active_users_queue);
    
    // Sends the given text message to the given group of recipients
    void SendTextMessage(const std::string& text, const std::vector<std::string>& recipients);
    // Receives incoming text messages via the sub_socket_
    void ReceiveMessages();

    // Calls SendHeartbeat() and ReceiveActiveUsers() functions one after another in a loop.
    // Should be run in a separate thread.
    void UpdateClientStatus();

    // Sends the client heartbeat message to the server to signal that it's online.
    // Returns the value of the req_socket_.send(heartbeat_msg) operation.
    // Must be called before ReceiveActiveUsers()
    zmq::send_result_t SendHeartbeat();

    // Receives (in a response to the heartbeat sent) a list of active users
    // and signals them to the Frontend via the active_users_queue_.
    // Must be called after SendHeartbeat() only if it returned a successful send result.
    zmq::recv_result_t ReceiveActiveUsers();
    zmq::send_result_t SendProtobufMessage(zmq::socket_t& socket, const google::protobuf::Message& msg, zmq::send_flags flags = zmq::send_flags::none);

    };
}
#endif  // CLIENT_BACKEND_HPP_