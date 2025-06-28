# Makefile for feature-alloc module
CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread -D_GNU_SOURCE
LDFLAGS = -pthread

SRCS = alloc.c
TEST_SRCS = test_alloc.c
OBJS = $(SRCS:.c=.o)

.PHONY: all test clean

all: liballoc.a

liballoc.a: $(OBJS)
	ar rcs $@ $^

test: test_alloc
	mkdir -p logs
	./test_alloc

test_alloc: $(TEST_SRCS) liballoc.a
	$(CC) $(CFLAGS) -o $@ $(TEST_SRCS) -L. -lalloc $(LDFLAGS)

clean:
	rm -f $(OBJS) liballoc.a test_alloc
	rm -f logs/alloc_trace.log

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
