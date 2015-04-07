CC	= gcc
SRC	= IniHandler.c
HDR	= IniHandler.h
BLD	= bld/
FLAGS	= -c
NAME	= IniHandler.o
OUTPUT	= -o ${BLD}${NAME}
RM	= rm -f

all:
	${CC} ${FLAGS} ${OUTPUT} ${SRC}

clean:
	${RM} ${BLD}${NAME}

