CC	= gcc
SRC	= IniHandler.c
HDR	= IniHandler.h
FLAGS	= -c
NAME	= inihandler.o
OUTPUT	= -o ${NAME}
RM	= rm -f

all: ${HDR} ${SRC}
	${CC} ${FLAGS} ${OUTPUT} ${SRC}

clean:
	${RM} ${NAME}

