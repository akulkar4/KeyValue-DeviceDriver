CFLAGS := -m64 -O2 -g -D_GNU_SOURCE -D_REENTRANT -W -I/usr/local/include
LDFLAGS := -m64 -lm

all: keyvalue.c
	$(CC) $(CFLAGS) -Wall -fPIC -c keyvalue.c
	$(CC) $(CFLAGS) -shared -Wl,-soname,libckv.so.1 -o libckv.so.1.0 keyvalue.o

install: libckv.so.1.0
	cp libckv.so.1.0 /usr/local/lib/libckv.so.1
	ln -fs /usr/local/lib/libckv.so.1 /usr/local/lib/libckv.so
	ln -fs /usr/local/lib/libckv.so.1 /usr/local/lib/libckv.so.1.0
	cp keyvalue.h  /usr/local/include


clean:
	rm -f *.so *.o


.PHONY: all clean
