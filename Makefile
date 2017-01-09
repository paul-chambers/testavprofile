
CPPFLAGS += -Wall -isystem /usr/include/ffmpeg
LDFLAGS += -lavcodec -lavformat -lavdevice -lavutil

testavprofile : testavprofile.cpp testavprofile.hpp optionparser.h
