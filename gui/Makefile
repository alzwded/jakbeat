CC = gcc
CFLAGS = -o

CXX = g++
CXXFLAGS = --std=gnu++14 -c -g -I. -I.. `fltk-config --cxxflags` -O0 -o

LD = g++ -o
LDFLAGS = `fltk-config --ldflags`

JAKBEATDEPENDS = parser.o tokenizer.o string_utils.o
MYOBJS = gui.o file.o window.o window_callbacks.o matrix_editor.o control.o logger.o save_model.o
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

tags:
	ctags -f tags --tag-relative=yes --fields=+a+f+i+k+l+m+n+s+S+z+t --extra=+f+q --c-kinds=+c+d+e+f+g+l+m+n+p+s+t+u+v+x --c++-kinds=+c+d+e+f+g+l+m+n+p+s+t+u+v+x *

clean:
	rm -f parser.cpp jakbeat-gui parser.y parser.h parser.out *.o tokenizer.cpp ../vendor/lemon/lemon

.PHONY: tags
