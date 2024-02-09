QT += widgets

SOURCES += client_frontend.cpp test.pb.cc simple_client.cpp

# Define project name and version
TARGET = client
VERSION = 1.0

# Tell qmake where to find header files
INCLUDEPATH += /path/to/your/protobuf/include
INCLUDEPATH += /path/to/your/zmq/include

# Tell qmake where to find library files
LIBS += -lprotobuf
LIBS += -lzmq
LIBS += -lboost_program_options
LIBS += -luuid


# Set the C++ standard
CONFIG += c++11

# Specify any other configuration options if needed

# Optionally, specify additional compiler flags
QMAKE_CXXFLAGS += -g

# Optionally, specify additional linker flags
QMAKE_LFLAGS += -g
