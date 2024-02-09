#include <string>
#include <QApplication>
#include <iostream>

#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include "chat_client_app.hpp"

namespace po = boost::program_options;

static inline bool is_not_alnum_space(const char c) {
    return !(isalnum(c) || (c == '_') || (c == '-'));
}

bool string_is_valid(const std::string &str) {
    return find_if(str.begin(), str.end(), is_not_alnum_space) == str.end();
}


int main(int argc, char* argv[]) {
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
    //qRegisterMetaType<QItemSelection>("QItemSelection");

    auto chat_controller = chat_app::ChatClientApp(user_name, full_server_push_address, full_server_sub_address, full_server_req_address);






    return app.exec();

}