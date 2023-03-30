#INCLUDES = -I../include
out:=server
#srcs:=main.c add.c sub.c
srcs:=$(wildcard *.c)
#objs:=main.o add.o sub.o
#objs:=$(srcs:%.c=%.o)
objs:=$(patsubst %.c,%.o,$(srcs))
CC:=gcc
$(out):$(objs)
	$(CC) $^ -o $@ -pthread -lcrypt
%.o:%.c
	$(CC) -c $^ -o $@ -lcrypt
.PHONY:clean rebuild
clean:
	$(RM) $(objs) $(out)
rebuild:clean main