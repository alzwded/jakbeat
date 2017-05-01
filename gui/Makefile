VERSION = 1

CC = gcc
CFLAGS = -o

CXX = g++
CXXFLAGS = --std=gnu++14 -c -DVERSION=$(VERSION) -g -I. -I.. `fltk-config --cxxflags` -o

LD = g++ -o
LDFLAGS = `fltk-config --ldflags`

JAKBEATDEPENDS = parser.o tokenizer.o
MYOBJS = gui.o file.o window.o window_callbacks.o matrix_editor.o
OBJS = $(MYOBJS) $(JAKBEATDEPENDS)
HEADERS = $(shell echo *.h)

jakbeat-gui: $(OBJS) $(HEADERS)
	$(LD) $@ $(OBJS) $(LDFLAGS)

parser.cpp: parser.y ../vendor/lemon/lemon
	../vendor/lemon/lemon parser.y
	mv parser.c parser.cpp    

parser.y: ../parser.y
	install -m 444 ../parser.y parser.y 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $@ $<

%.o: ../%.cpp
	$(CXX) $(CXXFLAGS) $@ $<

../vendor/lemon/lemon: ../vendor/lemon/lemon.c ../vendor/lemon/lempar.c
	$(CC) $(CFLAGS) ../vendor/lemon/lemon ../vendor/lemon/lemon.c

clean:
	rm -f parser.cpp jakbeat-gui parser.y parser.h parser.out *.o tokenizer.cpp ../vendor/lemon/lemon
