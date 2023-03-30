#INCLUDES = -I../include
out:=client
#srcs:=main.c add.c sub.c
srcs:=$(wildcard *.c)
#objs:=main.o add.o sub.o
#objs:=$(srcs:%.c=%.o)
objs:=$(patsubst %.c,%.o,$(srcs))
CC:=gcc
$(out):$(objs)
	$(CC) $^ -o $@ -pthread
%.o:%.c
	$(CC) -c $^ -o $@
.PHONY:clean rebuild
clean:
	$(RM) $(objs) $(out)
rebuild:clean main