#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <iostream>

#include "server.hpp"

namespace po = boost::program_options;


int main(int argc, char* argv[]) {
  std::string server_port_to_pull;
  std::string server_host;
  std::string server_pub_port;
  std::string heartbeat_port;
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "produce help message")(
      "server_address,a", po::value<std::string>(&server_host)->default_value("localhost"), "Server host address")(
      "pull_port,p", po::value<std::string>(&server_port_to_pull)->default_value("8000"),
      "Server port to pull messages")("sub_port,s",
                                      po::value<std::string>(&server_pub_port)->default_value("8001"),
                                      "Server port to publish messages")(
      "heartbeat_port,l", po::value<std::string>(&heartbeat_port)->default_value("8002"),
      "Server port to receive heartbeat messages");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }
  po::notify(vm);

  const std::string full_server_pull_address =
      std::string("tcp://") + server_host + std::string(":") + server_port_to_pull;
  const std::string full_server_pub_address =
      std::string("tcp://") + server_host + std::string(":") + server_pub_port;
  const std::string full_server_rep_address = std::string("tcp://") + server_host + std::string(":") + heartbeat_port;
  auto server = chat_app::Server(full_server_pull_address, full_server_pub_address, full_server_rep_address);
  server.Serve();

}