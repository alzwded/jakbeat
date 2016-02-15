VERSION = 1
CC = cl.exe
LEMONROOT = vendor\lemon
SDLROOT = vendor\SDL2-2.0.4
!IF DEFINED(JAKBEAT_OPTS) && "$(JAKBEAT_OPTS)" == "debug"
CFLAGS = /Od /c /EHa /I. /Zi /arch:SSE2 /DVERSION=$(VERSION) /I"$(SDLROOT)\include"
!ELSE
CFLAGS = /Ox /c /EHa /I. /arch:SSE2 /DVERSION=$(VERSION) /I"$(SDLROOT)\include"
!ENDIF
#CFLAGS = /c /EHsc /I. /Zi /arch:SSE2 /DVERSION=$(VERSION) /RTC1 /analyze  /Ge /GS /Gs
LD = link.exe
LDOPTS = /OUT:jakbeat.exe /DEBUG /PDB:jakbeat.pdb /LIBPATH:"$(SDLROOT)\lib\$(PLATFORM)"
LIBS = SDL2.lib

.SUFFIXES:.cpp .hpp .h .obj

OBJS = parser.obj tokenizer.obj file.obj render.obj wave.obj stereo.obj

jakbeat.exe: $(OBJS) SDL2.dll
	$(LD) $(LDOPTS) $(OBJS) $(LIBS)

SDL2.dll:
	copy $(SDLROOT)\lib\$(PLATFORM)\SDL2.dll

tokenizer.cpp: parser.h

parser.h: parser.cpp

parser.cpp: $(LEMONROOT)\lemon.exe parser.y
	$(LEMONROOT)\lemon.exe parser.y
	if EXIST parser.cpp del parser.cpp
	ren parser.c parser.cpp

{}.cpp{}.obj::
	$(CC) $(CFLAGS) $<

$(LEMONROOT)\lemon.exe: $(LEMONROOT)\lemon.c $(LEMONROOT)\lempar.c
	$(CC) /Fe:$(LEMONROOT)\lemon.exe $(LEMONROOT)\lemon.c

clean:
	del /q *.obj jakbeat.exe $(LEMONROOT)\lemon.exe parser.cpp parser.out parser.h *.ilk *.pdb SDL2.dll
