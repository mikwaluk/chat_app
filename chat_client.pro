QT += widgets

SOURCES += client_frontend.cpp test.pb.cc client_backend.cpp chat_client_app.cpp main.cpp

# Define project name and version
TARGET = client
VERSION = 1.0

# Tell qmake where to find library files
LIBS += -lprotobuf
LIBS += -lzmq
LIBS += -lboost_program_options


# Set the C++ standard
CONFIG += c++17

# Specify any other configuration options if needed

# Optionally, specify additional compiler flags
QMAKE_CXXFLAGS += -g

# Optionally, specify additional linker flags
QMAKE_LFLAGS += -g
