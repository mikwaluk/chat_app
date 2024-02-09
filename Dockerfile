FROM ubuntu:22.04

RUN apt-get update
RUN apt-get install -y vim git build-essential
RUN apt-get install -y cmake
RUN groupadd -g 1000 nonroot && \
    useradd -ms /bin/bash -r -u 1000 -g nonroot nonroot

USER nonroot
WORKDIR /home/nonroot/git_installs
RUN git clone https://github.com/zeromq/libzmq.git /home/nonroot/git_installs/libzmq
RUN ls
RUN pwd
WORKDIR /home/nonroot/git_installs/libzmq/build
RUN pwd
USER root
RUN cmake .. && make install
USER nonroot

RUN git clone https://github.com/zeromq/cppzmq.git /home/nonroot/git_installs/cppzmq
WORKDIR /home/nonroot/git_installs/cppzmq/build
USER root
RUN cmake .. && make install

USER nonroot
WORKDIR /home/nonroot
RUN git clone https://github.com/mikwaluk/chat_app.git
WORKDIR /home/nonroot/chat_app
USER root
RUN apt-get install -y libboost-program-options-dev libprotobuf-dev protobuf-compiler qtbase5-dev qt5-qmake
USER nonroot
RUN protoc -I=. --cpp_out=. ./messages.proto
RUN mv messages.pb.cc src/
RUN mv messages.pb.h include/
WORKDIR /home/nonroot/chat_app/build
RUN qmake .. && make
WORKDIR /home/nonroot/chat_app
RUN g++ -g src/utils.cpp src/user_manager.cpp src/server.cpp src/messages.pb.cc src/main_server.cpp -lzmq -lprotobuf -o build/server -lboost_program_options -Iinclude/
USER root
RUN cp build/client build/server /usr/bin
USER nonroot