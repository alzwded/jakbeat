VERSION = 1

JAKBEAT-DEPS = parser.cpp ../tokenizer.cpp
SOURCES = gui.cpp file.cpp window.cpp window_callbacks.cpp
OBJS = gui.o file.o window.o window_callbacks.o parser.o tokenizer.o
HEADERS = $(shell echo *.h)

jakbeat-gui: $(OBJS) $(HEADERS)
	g++ -o jakbeat-gui $(OBJS) `fltk-config --ldflags`

parser.cpp: parser.y ../vendor/lemon/lemon
	../vendor/lemon/lemon parser.y
	mv parser.c parser.cpp    

parser.y: ../parser.y
	install -m 444 ../parser.y parser.y 

tokenizer.cpp: ../tokenizer.cpp
	install -m 444 ../tokenizer.cpp tokenizer.cpp

%.o: %.cpp
	g++ --std=gnu++14 -c -o $@ -DVERSION=$(VERSION) -g -I. -I.. `fltk-config --cxxflags` $<

../vendor/lemon/lemon: ../vendor/lemon/lemon.c ../vendor/lemon/lempar.c
	gcc -o ../vendor/lemon/lemon ../vendor/lemon/lemon.c

clean:
	rm -f parser.cpp jakbeat-gui parser.y parser.h parser.out *.o tokenizer.cpp
