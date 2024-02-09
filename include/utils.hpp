#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <string>
#include <zmq.hpp>
#include <google/protobuf/message.h>

namespace chat_app {
namespace utils {

bool IsCharInvalidForUsername(const char c);

bool IsValidUsername(const std::string& str);

zmq::send_result_t SendProtobufMessage(zmq::socket_t& socket, const google::protobuf::Message& msg,
                                                    zmq::send_flags flags = zmq::send_flags::none);

zmq::recv_result_t ReceiveProtobufMessage(zmq::socket_t& socket, google::protobuf::Message& msg,
                                                 zmq::recv_flags flags = zmq::recv_flags::none);
}  // namespace utils
}  // namespace chat_app

#endif  // UTILS_HPP_