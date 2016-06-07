CC = clang
CFLAGS = -g
LDLIBS  = -lm
#DEBUG = -DBINARYDEBUG

SRCS = linkedlist.c main.c talloc.c tokenizer.c parser.c interpreter.c primitives.c
HDRS = linkedlist.h value.h talloc.h tokenizer.h parser.h interpreter.h primitives.h
OBJS = $(SRCS:.c=.o)

interpreter: $(OBJS)
	$(CC) -rdynamic $(CFLAGS) $^  -o $@ $(LDLIBS)

%.o : %.c $(HDRS)
	$(CC)  $(CFLAGS) $(DEBUG) -c $<  -o $@

clean:
	rm *.o
	rm interpreter