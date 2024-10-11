CC = gcc
CFLAGS-common = -std=gnu18 -Wall -Wextra -Werror -pedantic
CFLAGS = $(CFLAGS-common) -O2
CFLAGS-dbg = $(CFLAGS-common) -Og -ggdb

TARGET = wsh
SRC = $(TARGET).c
HEADER = $(TARGET).h

LOGIN = areeb
SUBMITPATH = ~cs537-1/handin/$(LOGIN)

all: $(TARGET) $(TARGET)-dbg

$(TARGET): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) $< -o $@

$(TARGET)-dbg: $(SRC) $(HEADER)
	$(CC) $(CFLAGS-dbg) $< -o $@

clean:
	rm -f $(TARGET) $(TARGET)-dbg dict envtest cqueue

dict: dict.c dict.h
	$(CC) $(CFLAGS) $< -o $@

envtest: envtest.c dict.c dict.h
	$(CC) $(CFLAGS) $^ -o $@

cqueue: cqueue.c
	$(CC) $(CFLAGS) $< -o $@

submit: clean
	rm -rf $(SUBMITPATH)/p3;\
	mkdir $(SUBMITPATH)/p3;\
	cd ../;\
	cp -r . $(SUBMITPATH)/p3/;\
	cd solution

test:
	cd ../tests/;\
	./run-tests.sh;\
	cd ../solution/
