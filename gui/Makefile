VERSION = 1

jakbeat-gui: gui.cpp
	g++ -o jakbeat-gui -DVERSION=$(VERSION) -g -I. -I.. `fltk-config --cxxflags` gui.cpp parser.cpp ../tokenizer.cpp ../file.cpp ../stereo.cpp `fltk-config --libs`

parser.cpp: parser.y ../vendor/lemon/lemon
	../vendor/lemon/lemon parser.y
	mv parser.c parser.cpp    

parser.y: ../parser.y
	cp ../parser.y .

../vendor/lemon/lemon: ../vendor/lemon/lemon.c ../vendor/lemon/lempar.c
	gcc -o ../vendor/lemon/lemon ../vendor/lemon/lemon.c

clean:
	rm -f parser.cpp jakbeat-gui parser.y parser.h parser.out
