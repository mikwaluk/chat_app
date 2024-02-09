#include <zmq.hpp>
#include <string>
#include <thread>
#include <iostream>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <vector>

#include "test.pb.h"
namespace po = boost::program_options;
#include "client_backend.hpp"
#include <algorithm>
#include "client_frontend.hpp"
#include "chat_client_app.hpp"

#include <QApplication>
#include <QMetaType>

static inline bool is_not_alnum_space(const char c) {
    return !(isalnum(c) || (c == '_') || (c == '-'));
}

bool string_is_valid(const std::string &str) {
    return find_if(str.begin(), str.end(), is_not_alnum_space) == str.end();
}

namespace chat_app {
  ClientBackend::ClientBackend(const std::string& name, const std::string& push_addr, const std::string& sub_addr, const std::string& req_addr, std::queue<std::tuple<const std::string, const std::string>>* received_messages_queue, std::queue<std::shared_ptr<std::vector<std::string>>>* active_users_queue)
    : push_socket_ {ctx_, zmq::socket_type::push}
    , sub_socket_ {ctx_, zmq::socket_type::sub}
    , req_socket_ {ctx_, zmq::socket_type::req}
    , name_ {name}
    , stop_ {false}
    , incoming_msgs_queue_ {received_messages_queue}
    , active_users_queue_ {active_users_queue}
  {
    push_socket_.connect(push_addr);
    sub_socket_.set(zmq::sockopt::subscribe, "/all/");
    sub_socket_.set(zmq::sockopt::subscribe, ("/" + name_ + "/").c_str());
    sub_socket_.connect(sub_addr);
    req_socket_.connect(req_addr);
    t1 = std::thread(&ClientBackend::ReceiveMessages, this);
    t2 = std::thread(&ClientBackend::SendHeartbeat, this);
  }

  void ClientBackend::SendTextMessage(const std::string& text, const std::vector<std::string>& recipients) {
    auto msg = test::TestMsg();
    msg.set_from(name_);
    for (const auto& recipient : recipients) {
      msg.add_to(recipient);
    }
    msg.set_message_text(text);

    std::string serialized_output;
    msg.SerializeToString(&serialized_output);
    zmq::message_t request (serialized_output.size());
    memcpy ((void *) request.data (), serialized_output.c_str(), serialized_output.size());
    const auto result = push_socket_.send(request, zmq::send_flags::none);
    std::cout << "Send result: " << result.value() << "\n";
  }

  void ClientBackend::ReceiveMessages() {
    std::cout << "Start receiving...\n";
    while (!stop_) {
      zmq::message_t topic;
      zmq::recv_result_t recv_result = sub_socket_.recv(topic, zmq::recv_flags::dontwait);
      if (!recv_result.has_value()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }
      int64_t more = sub_socket_.get(zmq::sockopt::rcvmore);
      if (more != 1) {
        std::cout << "Error receiving mesage - unexpected number of remaining message parts. Expected 1, got " << more << "\n";
        continue;
      }
      zmq::message_t rec_data;
      sub_socket_.recv(rec_data);
      test::TestMsg rec_msg;
      rec_msg.ParseFromArray(rec_data.data(), rec_data.size());
      if (rec_msg.from() == name_) {
        // Drop your own message
        continue;
      }
      std::cout << "[" << name_ << "]: Received message \"" << rec_msg.message_text() << "\" on topic " << topic.to_string() << " from " << rec_msg.from() << "\n";
      incoming_msgs_queue_->push(std::make_tuple(rec_msg.from(), rec_msg.message_text()));
    }
  }

  void ClientBackend::SendHeartbeat() {
    while (!stop_) {
      auto msg = test::HeartbeatMsg();
      msg.set_from(name_);
      std::string serialized_output;
      msg.SerializeToString(&serialized_output);
      zmq::message_t request (serialized_output.size());
      memcpy ((void *) request.data (), serialized_output.c_str(), serialized_output.size());
      // TODO - what if we don't get any reply? Server is down
      std::cout << "send heartbeat\n";
      req_socket_.send(request, zmq::send_flags::none);
      std::cout << "sent heartbeat\n";
      zmq::message_t rec_data;
      req_socket_.recv(rec_data);
        std::cout << "received heartbeat reply\n";
      test::HeartbeatMsg rec_msg;
      rec_msg.ParseFromArray(rec_data.data(), rec_data.size());
      
        std::cout << "ParsedFromArray\n";
      std::cout << "Active users:\n";
      for (const auto& user : rec_msg.active_users()) {
        std::cout << user << "\n";
      }
      std::cout << "size of vector: " << rec_msg.active_users().size() << "\n";
      auto active_users = std::make_shared<std::vector<std::string>>();
      for (const auto& user : rec_msg.active_users()) {
        active_users->push_back(user);
      }
      //active_users_queue_->push(std::vector<std::string>{rec_msg.active_users().begin(), rec_msg.active_users().end()});
      std::cout << "try to push to the queue...\n";
      std::cout << "queue size\n";
      std::cout << active_users_queue_->size() << "\n";
      active_users_queue_->push(active_users);
      std::cout << "pushed\n";
      std::cout << "\n";
      std::cout <<"sleep\n";
      std::this_thread::sleep_for(std::chrono::seconds(1));

    }
  }
  void ClientBackend::Stop() {
    std::cout << "Stopping...\n";
    stop_ = true;
    t1.join();
    t2.join();
    std::cout << "Stopped!\n";
  }

} // namespace chat_app

void send_some_messages(chat_app::ClientBackend& client) {
    auto empty_v = std::vector<std::string> {};
    client.SendTextMessage("test1", empty_v);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.SendTextMessage("test2", empty_v);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.SendTextMessage("test3", empty_v);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.SendTextMessage("test4", empty_v);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.SendTextMessage("test5", empty_v);
    client.Stop();
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    std::string server_port_to_push;
    std::string server_host;
    std::string user_name;
    std::string server_port_to_subscribe;
    std::string heartbeat_port;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("user_name,u", po::value<std::string>(&user_name)->required(), "Name of the user (required)")
        ("server_address,a", po::value<std::string>(&server_host)->default_value("localhost"), "Server host address")
        ("push_port,p", po::value<std::string>(&server_port_to_push)->default_value("8000"), "Server port to push messages")
        ("sub_port,s", po::value<std::string>(&server_port_to_subscribe)->default_value("8001"), "Server port to subscribe for messages")
        ("heartbeat_port,l", po::value<std::string>(&heartbeat_port)->default_value("8002"), "Server port to send heartbeat messages")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
  

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }
    po::notify(vm);
    if (!string_is_valid(user_name)) {
      std::cout << "Username must contain only alphanumeric characters, dashes or underscores!\n";
      return 1;
    }
    if (user_name == std::string("all")) {
      std::cout << "The user name must not be \"all\"\n";
      return 1;
    }
    const std::string full_server_push_address = std::string("tcp://") + server_host + std::string(":") + server_port_to_push;
    const std::string full_server_sub_address = std::string("tcp://") + server_host + std::string(":") + server_port_to_subscribe;
    const std::string full_server_req_address = std::string("tcp://") + server_host + std::string(":") + heartbeat_port;

    QApplication app(argc, argv);
    qRegisterMetaType<QItemSelection>("QItemSelection");

    auto chat_controller = chat_app::ChatClientApp(user_name, full_server_push_address, full_server_sub_address, full_server_req_address);






    return app.exec();

}