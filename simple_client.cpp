#include <zmq.hpp>
#include <string>
#include <thread>
#include <iostream>
#include <uuid/uuid.h>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <vector>

#include "test.pb.h"
namespace po = boost::program_options;
#include "simple_client.hpp"
#include <algorithm>
#include "client_frontend.hpp"
#include "chat_controller.hpp"

#include <QApplication>

static inline bool is_not_alnum_space(const char c) {
    return !(isalnum(c) || (c == '_') || (c == '-'));
}

bool string_is_valid(const std::string &str) {
    return find_if(str.begin(), str.end(), is_not_alnum_space) == str.end();
}

namespace chat_app {
  SimpleClient::SimpleClient(const std::string& name, const std::string& push_addr, const std::string& sub_addr)
    : push_socket_ {ctx_, zmq::socket_type::push}
    , sub_socket_ {ctx_, zmq::socket_type::sub}
    , name_ {name}
    , stop_ {false}
  {
    push_socket_.connect(push_addr);
    sub_socket_.set(zmq::sockopt::subscribe, "/all/");
    sub_socket_.set(zmq::sockopt::subscribe, ("/" + name_ + "/").c_str());
    sub_socket_.connect(sub_addr);
    t1 = std::thread(&SimpleClient::ReceiveMessage, this);
  }

  void SimpleClient::SendTextMessage(const std::string& text, const std::vector<std::string>& recipients) {
    auto msg = test::TestMsg();
    msg.set_from(name_);
    for (const auto& recipient : recipients) {
      msg.add_to(recipient);
    }
    msg.set_name(text);

    uuid_t uuid_c_struct;
    uuid_generate_time(uuid_c_struct);
    char uuid_str[37];
    uuid_unparse(uuid_c_struct, uuid_str);
    auto uuid_proto = test::UUID();
    uuid_proto.set_value(uuid_str);
    msg.mutable_uuid()->CopyFrom(uuid_proto);

    std::string serialized_output;
    msg.SerializeToString(&serialized_output);
    zmq::message_t request (serialized_output.size());
    memcpy ((void *) request.data (), serialized_output.c_str(), serialized_output.size());
    const auto result = push_socket_.send(request, zmq::send_flags::none);
    std::cout << "Send result: " << result.value() << "\n";
  }

  void SimpleClient::ReceiveMessage() {
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
      std::cout << "[" << name_ << "]: Received message \"" << rec_msg.name() << "\" on topic " << topic.to_string() << " from " << rec_msg.from() << "\n";
    }
  }

  void SimpleClient::Stop() {
    std::cout << "Stopping...\n";
    stop_ = true;
    t1.join();
    std::cout << "Stopped!\n";
  }

  SimpleClient::~SimpleClient() {
    std::cout << "Closing sub socket\n";
    sub_socket_.close();
    std::cout << "Closing push socket\n";
    push_socket_.close();
    std::cout << "Closing context\n";
    ctx_.close();
  }

} // namespace chat_app

void send_some_messages(chat_app::SimpleClient& client) {
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
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("user_name,u", po::value<std::string>(&user_name)->required(), "Name of the user (required)")
        ("server_address,a", po::value<std::string>(&server_host)->default_value("localhost"), "Server host address")
        ("push_port,p", po::value<std::string>(&server_port_to_push)->default_value("8000"), "Server port to push messages")
        ("sub_port,s", po::value<std::string>(&server_port_to_subscribe)->default_value("8001"), "Server port to subscribe for messages")
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


    QApplication app(argc, argv);

    auto chat_controller = chat_app::ChatController(user_name, full_server_push_address, full_server_sub_address);






    return app.exec();

}