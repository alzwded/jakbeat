VERSION = 1

JAKBEAT-DEPS = parser.cpp ../tokenizer.cpp ../stereo.cpp 
SOURCES = gui.cpp file.cpp

jakbeat-gui: gui.cpp parser.cpp
	g++ --std=gnu++14 -o jakbeat-gui -DVERSION=$(VERSION) -g -I. -I.. `fltk-config --cxxflags` $(SOURCES) $(JAKBEAT-DEPS) `fltk-config --libs`

parser.cpp: parser.y ../vendor/lemon/lemon
	../vendor/lemon/lemon parser.y
	mv parser.c parser.cpp    

parser.y: ../parser.y
	cp ../parser.y .

../vendor/lemon/lemon: ../vendor/lemon/lemon.c ../vendor/lemon/lempar.c
	gcc -o ../vendor/lemon/lemon ../vendor/lemon/lemon.c

clean:
	rm -f parser.cpp jakbeat-gui parser.y parser.h parser.out
