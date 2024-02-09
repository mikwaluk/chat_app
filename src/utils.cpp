#include <ctype.h>
#include <google/protobuf/message.h>

#include <algorithm>
#include <string>
#include <zmq.hpp>

namespace chat_app {
namespace utils {
bool IsCharInvalidForUsername(const char c) { return !(isalnum(c) || (c == '_') || (c == '-')); }

bool IsValidUsername(const std::string& str) {
  return find_if(str.begin(), str.end(), IsCharInvalidForUsername) == str.end();
}

// Helper function to serialize and send a given Protobuf message via a ZMQ socket.
zmq::send_result_t SendProtobufMessage(zmq::socket_t& socket, const google::protobuf::Message& msg,
                                       zmq::send_flags flags) {
  std::string serialized_output;
  msg.SerializeToString(&serialized_output);
  zmq::message_t zmq_msg{serialized_output.size()};
  memcpy((void*)zmq_msg.data(), serialized_output.c_str(), serialized_output.size());
  const auto result = socket.send(zmq_msg, flags);
  return result;
}

zmq::recv_result_t ReceiveProtobufMessage(zmq::socket_t& socket, google::protobuf::Message& msg,
                                          zmq::recv_flags flags = zmq::recv_flags::none) {
  zmq::message_t zmq_msg;
  const auto recv_result = socket.recv(zmq_msg, flags);
  if (!recv_result.has_value()) {
    return recv_result;
  }
  msg.ParseFromArray(zmq_msg.data(), zmq_msg.size());
  return recv_result;
}

}  // namespace utils
}  // namespace chat_app