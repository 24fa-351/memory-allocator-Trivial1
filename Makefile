CC = gcc
CFLAGS = -Wall -g -pthread

all: system_heap my_heap

system_heap: test_heap_manager.c
	$(CC) $(CFLAGS) -DUSE_SYSTEM_MALLOC -o system_heap test_heap_manager.c

my_heap: test_heap_manager.c heap_manager.c
	$(CC) $(CFLAGS) -o my_heap test_heap_manager.c heap_manager.c

clean:
	rm -f system_heap my_heap
