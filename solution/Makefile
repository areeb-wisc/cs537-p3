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
	rm -f $(TARGET) $(TARGET)-dbg

submit: clean
	rm -rf $(SUBMITPATH)/p3;\
	mkdir $(SUBMITPATH)/p3;\
	cd ../;\
	cp -r . $(SUBMITPATH)/p3/;\
	cd solution

test:
	cd ../tests/;\
	chmod +x run-tests.sh;\
	./run-tests.sh -c;\
	cd ../solution/
