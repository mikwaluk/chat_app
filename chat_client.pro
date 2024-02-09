QT += widgets

SOURCES += src/messages.pb.cc
SOURCES += src/utils.cpp src/client_frontend.cpp src/client_backend.cpp src/chat_client_controller.cpp src/main_client.cpp

INCLUDEPATH+=include/

TARGET = client
VERSION = 1.0

LIBS += -lprotobuf
LIBS += -lzmq
LIBS += -lboost_program_options

CONFIG += c++17

QMAKE_CXXFLAGS += -g

QMAKE_LFLAGS += -g
