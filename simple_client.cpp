#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <string>
#include <thread>
#include <iostream>
#include <uuid/uuid.h>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <vector>

#include "test.pb.h"
namespace po = boost::program_options;

class SimpleClient {
  private:
  zmq::context_t ctx_;
  zmq::socket_t push_socket_;
  zmq::socket_t sub_socket_;
  std::string name_;
  public:
  SimpleClient(const std::string& name, const std::string& push_addr, const std::string& sub_addr): push_socket_ {ctx_, zmq::socket_type::push}, sub_socket_ {ctx_, zmq::socket_type::sub}, name_ {name} {
    push_socket_.connect(push_addr);
    sub_socket_.set(zmq::sockopt::subscribe, "/all");
    sub_socket_.set(zmq::sockopt::subscribe, "/" + name_);
    sub_socket_.connect(sub_addr);
  }
  void SendTextMessage(const std::string& text, const std::vector<std::string>& recipients) {
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
  
};

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
    const std::string full_server_push_address = std::string("tcp://") + server_host + std::string(":") + server_port_to_push;
    const std::string full_server_sub_address = std::string("tcp://") + server_host + std::string(":") + server_port_to_subscribe;
    auto client = SimpleClient(user_name, full_server_push_address, full_server_sub_address);

    auto empty_v = std::vector<std::string> {};
    client.SendTextMessage("test1", empty_v);

}