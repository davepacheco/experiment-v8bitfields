#
# basic Makefile
#

CFLAGS = -Wall -Werror -Wpedantic -std=c99

v8bitfield: v8bitfield.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

clean:
	rm -f v8bitfield
