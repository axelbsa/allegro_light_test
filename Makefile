CC=g++
# OPT = -static -mwindows -static-libgcc -std=gnu++11
OPT = -mwindows
OUT = light

SRC = main.cpp
LIBS =	-lallegro_monolith-static \
		-lwinmm -lpsapi -lshlwapi \
		-lopengl32 -lole32 \
		-lpng16 -ljpeg -lzlib \

#%.o: %.c $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

$(OUT): $(SRC)
	$(CC) -o $@ $^ $(OPT) $(LIBS)

clean:
	del *.o *.exe
