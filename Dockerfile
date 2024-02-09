FROM ubuntu:22.04

RUN apt-get update
RUN apt-get install -y vim git build-essential
RUN apt-get install -y cmake

WORKDIR /home/root/git_installs
RUN git clone https://github.com/zeromq/libzmq.git /home/root/git_installs/libzmq
RUN ls
RUN pwd
WORKDIR /home/root/git_installs/libzmq/build
RUN pwd
RUN cmake .. && make install

RUN git clone https://github.com/zeromq/cppzmq.git /home/root/git_installs/cppzmq
WORKDIR /home/root/git_installs/cppzmq/build

RUN cmake .. && make install

WORKDIR /home/root
RUN ls
RUN git clone https://github.com/mikwaluk/chat_app.git
WORKDIR /home/root/chat_app

RUN apt-get install -y libboost-program-options-dev libprotobuf-dev protobuf-compiler qtbase5-dev qt5-qmake

RUN protoc -I=. --cpp_out=. ./messages.proto
RUN mv messages.pb.cc src/
RUN mv messages.pb.h include/
WORKDIR /home/root/chat_app/build
RUN qmake .. && make
WORKDIR /home/root/chat_app
RUN g++ -g src/utils.cpp src/user_manager.cpp src/server.cpp src/messages.pb.cc src/main_server.cpp -lzmq -lprotobuf -o build/server -lboost_program_options -Iinclude/

RUN cp build/client build/server /usr/bin
