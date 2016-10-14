###########################################
# he2325u interface program
#
# Rainer Wetzel
# 2011-02-18
###########################################

all: he2325u

UNAME := $(shell uname)

CC=gcc
CXX=g++
CPPOBJS=./he2325u.o
OBJS=$(COBJS) $(CPPOBJS)
CFLAGS+=-Wall -g -c
ifeq ($(UNAME), Linux)
LIBS=-lhidapi-libusb
endif
ifeq ($(UNAME), Darwin)
COBJS=./hidapi/mac/hid.o
LIBS=`pkg-config libusb-1.0 --libs` -framework CoreFoundation -framework IOKit
endif


he2325u: $(OBJS)
	g++ -Wall -g $^ $(LIBS) -o he2325u

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJS) he2325u

.PHONY: clean
