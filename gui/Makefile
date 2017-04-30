VERSION = 1

JAKBEAT-DEPS = parser.cpp ../tokenizer.cpp
SOURCES = gui.cpp file.cpp window.cpp window_callbacks.cpp
HEADERS = $(shell echo *.h)

jakbeat-gui: $(SOURCES) $(JAKBEAT-DEPS) $(HEADERS)
	g++ --std=gnu++14 -o jakbeat-gui -DVERSION=$(VERSION) -g -I. -I.. `fltk-config --cxxflags` $(SOURCES) $(JAKBEAT-DEPS) `fltk-config --ldflags`

parser.cpp: parser.y ../vendor/lemon/lemon
	../vendor/lemon/lemon parser.y
	mv parser.c parser.cpp    

parser.y: ../parser.y
	cp ../parser.y .

../vendor/lemon/lemon: ../vendor/lemon/lemon.c ../vendor/lemon/lempar.c
	gcc -o ../vendor/lemon/lemon ../vendor/lemon/lemon.c

clean:
	rm -f parser.cpp jakbeat-gui parser.y parser.h parser.out
