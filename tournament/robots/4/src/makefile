CFLAGS= -O3 -ansi -pedantic -static

all: runme runme16 runme20

runme-fork: play.c
	cc ${CFLAGS} -DPARALLEL -DMAXN=8 play.c -o runme-fork

runme: play.c
	cc ${CFLAGS} -DMAXN=8 play.c -o runme

runme16: play.c
	cc ${CFLAGS} -DMAXN=16 play.c -o runme16

runme20: play.c
	cc ${CFLAGS} -DMAXN=20 play.c -o runme20

install: all
	strip runme16 runme20 runme
	mv runme16 runme20 ../support
	mv runme ..
