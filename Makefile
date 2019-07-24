# (C)2013, Bruno Keymolen
# http://www.keymolen.com
# http://www.keymolen.com/2013/05/hough-transformation-c-implementation.html
CXX=g++
CC=gcc
OPTFLAGS=-g3 -ggdb -O0
CXXFLAGS=-Wall --std=c++11 -I. -I/opt/keymolen/include -I/usr/local/include 
CFLAGS=-Wall $(OPTFLAGS)
LDFLAGS= -L/opt/keymolen/lib -L/usr/local/lib $(OPTFLAGS)


LIBRARIES=opencv libavformat libavcodec libswscale


CXXFLAGS+= `PKG_CONFIG_PATH=/opt/keymolen/lib/pkgconfig pkg-config $(LIBRARIES) --cflags`
LDFLAGS+= `PKG_CONFIG_PATH=/opt/keymolen/lib/pkgconfig pkg-config $(LIBRARIES) --libs`




SRC = main.o vdecoder.o
        
all: decoder


decoder: $(SRC) 
	$(CXX) $(SRC) $(LDFLAGS) -o decoder 


%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


%.o: %.cpp %.hpp
	$(CXX) $(OPTFLAGS) $(CXXFLAGS) -c -o $@ $<


%.o: %.cpp
	$(CXX) $(OPTFLAGS) $(CXXFLAGS) -c -o $@ $<


clean:
	rm -f *.o decoder


PREFIX ?= /usr


install: all
	install -d $(PREFIX)/bin
	install decoder  $(PREFIX)/bin


dependencies:
	sudo apt install libopencv-dev




.PHONY: clean all decoder install

