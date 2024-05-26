CC=gcc
CFLAGS=-g -O3 -Wall -Wno-unused-function -Wno-pointer-sign \
       -I. -Isecp256k1 -Isecp256k1/include -funsafe-loop-optimizations
LDFLAGS=$(CFLAGS)
LDLIBS=-lm -lgmp -lssl -lcrypto -lpthread

SHA256=sha256/sha256.o sha256/sha256-avx-asm.o sha256/sha256-avx2-asm.o \
       sha256/sha256-ssse3-asm.o sha256/sha256-ni-asm.o

OBJS=vanitygen.o base58.o cpu.o rmd160.o segwit_addr.o $(SHA256)


all: vanitygen

install: all
	cp --remove-destination -p vanitygen /usr/local/bin/

clean:
	rm -f vanitygen *.o sha256/*.o

distclean: clean
	$(MAKE) -C secp256k1 distclean


vanitygen: $(OBJS)


UNAME_S := $(shell uname -s)

# If on macOS, please install secp256k1 using (home)brew instead.
$(OBJS): Makefile *.h $(ifneq ($(UNAME_S),Darwin),secp256k1/src/libsecp256k1-config.h secp256k1/src/ecmult_static_context.h)

secp256k1/src/libsecp256k1-config.h:
	(cd secp256k1;./autogen.sh;./configure)

secp256k1/src/ecmult_static_context.h:
	$(MAKE) -C secp256k1 src/ecmult_static_context.h