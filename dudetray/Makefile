PKG=`pkg-config --libs --cflags gtk+-2.0`
OPTIMIZATIONS=-O
DEBUG_OPTS=-Wall -g

all: clean dudetray

debug:
	make OPTIMIZATIONS="${DEBUG_OPTS}"

dudetray: 
	cc ${OPTIMIZATIONS} -o dudetray dudetray.c ${PKG}

clean:
	rm -f dudetray
