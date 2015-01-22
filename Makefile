CC	= gcc
SRC	= IniReader.c
HDR	= IniReader.h
FLAGS	= -c
NAME	= inireader.o
OUTPUT	= -o ${NAME}
RM	= rm -f

all: ${HDR} ${SRC}
	${CC} ${FLAGS} ${OUTPUT} ${SRC}

clean:
	${RM} ${NAME}

