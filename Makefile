CC=g++
# OPT = -static -mwindows -static-libgcc -std=gnu++11
OUT = light
DYN_OUT = light_dynamic
SRC = main.cpp

PLATFORM = $(shell uname)

# Predefine for windows that not always returns the same uname
# Static Windows
OPT = -mwindows -static-libgcc
LIBS =	-lallegro_monolith-static \
		-lallegro_font-static \
		-lwinmm -lpsapi -lshlwapi \
		-lopengl32 -lole32 \
		-lpng16 -ljpeg -lzlib

# Dynamic windows
DYN_LIBS = -lallegro_monolith

# Static Linux
ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	OPT = -static-libgcc -std=gnu++11
	LIBS= -lallegro -lallegro_image \
			-lallegro_font -lallegro_ttf \
			-lallegro_dialog
endif

#%.o: %.c $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

$(OUT): $(SRC)
	$(CC) -o $@ $^ $(OPT) $(LIBS)

dyn: $(SRC)
	$(CC) -o $(DYN_OUT) $^ $(OPT) $(DYN_LIBS)

clean:
	del *.o *.exe


# g++ -static-libgcc -Wall -std=gnu++11 main.cpp 
