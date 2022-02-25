MAKE = make
OBJECTS = main.o
LIBS = 
TARGET_UNIX = pacman3d
TARGET_WIN32 = pacman3d.exe
TARGET = 
CC = gcc
STRIP = strip
RM_UNIX = rm
RM_WIN32 = del
RM = 
LINK_FLAGS_UNIX = -lglut -lGLU
LINK_FLAGS_WIN32 = -mwindows glut32.lib -lopengl32 -lglu32
LINK_FLAGS =
CFLAGS_UNIX = -DUNIX
CFLAGS_WIN32 = -DWINDOWS
CFLAGS =
DEBUG_FLAGS = "-Wall -g"
RELEASE_FLAGS = "-Wall -O3"

ifdef SystemRoot
	LINK_FLAGS = $(LINK_FLAGS_WIN32)
	CFLAGS = $(CFLAGS_WIN32)
	TARGET = $(TARGET_WIN32)
	RM = del -Q 
	FixPath = $(subst /,\,$1)
else
	ifeq ($(shell uname), Linux)
		LINK_FLAGS = $(LINK_FLAGS_UNIX)
		CFLAGS = $(CFLAGS_UNIX)
		TARGET = $(TARGET_UNIX)
		RM = rm -f
		FixPath = $1
	endif
endif

default : debug
debug :
	@${MAKE} targets BUILD_FLAGS=$(DEBUG_FLAGS)
release :
	@${MAKE} targets BUILD_FLAGS=$(RELEASE_FLAGS)
targets : $(OBJECTS)
	$(CC) -o $(TARGET) $(BUILD_FLAGS) $(OBJECTS) $(LINK_FLAGS) $(LIBS)
main.o : main.c 
	$(CC) $(BUILD_FLAGS) $(CFLAGS) -o main.o -c main.c

.PHONY: clean cleanobj
clean :
	$(RM) $(OBJECTS)
	$(RM) $(TARGET)
cleanobj :
	$(RM) $(OBJECTS)

