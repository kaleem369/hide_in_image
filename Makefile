COMPILER_PREFIX=
#COMPILER_PREFIX=i686-w64-mingw32-

#CC=gcc
CC=$(COMPILER_PREFIX)gcc
CFLAGS=-O2 -std=c99 -Wall -flto
#CFLAGS += -DNDEBUG
#LD=g++
LD=$(COMPILER_PREFIX)g++
LDFLAGS=-O2 -Wall -flto
AR=$(COMPILER_PREFIX)ar
RANLIB=$(COMPILER_PREFIX)ranlib

# is ar, ranlib platform specific? yes.
My_make_flags=CC=$(CC) LD=$(LD) AR=$(AR) RANLIB=$(RANLIB)

LIBDIR=-L optipng/src/opnglib/ -L optipng/src/libpng/ -L optipng/src/zlib/ -L optipng/src/pngxtern/ -L optipng/src/minitiff/ -L optipng/src/gifread/ -L optipng/src/pnmio/ -L optipng/src/opngreduc/
LIBS=-lopng -lpng -lz -lpngxtern -lminitiff -lgifread -lpnmio -lopngreduc


all: hii

hii: main.o libhii.a \
  optipng/src/pngxtern/libpngxtern.a \
  optipng/src/minitiff/libminitiff.a \
  optipng/src/pnmio/libpnmio.a \
  optipng/src/gifread/libgifread.a \
  optipng/src/libpng/libpng.a \
  optipng/src/zlib/libz.a \
  optipng/src/opngreduc/libopngreduc.a \
  optipng/src/opnglib/libopng.a
	$(LD) $(LDFLAGS) main.o -o hii -flto -s -L. -lhii $(LIBDIR) $(LIBS)

main.o: main.c libhii.h
	$(CC) $(CFLAGS) -c main.c

libhii.a: libhii.o
	$(AR) cru libhii.a libhii.o
	$(RANLIB) libhii.a

libhii.o: libhii.c libhii.h stb_image_write.h stb_image.h
	$(CC) $(CFLAGS) -I optipng/src/opnglib/include -c libhii.c

optipng/%.a: optipng/Makefile optipng/src/libpng/pnglibconf.h
	cd optipng && \
	$(MAKE) $(My_make_flags) && \
	cd ..

optipng/Makefile:
	cd optipng && \
	env $(My_make_flags) ./configure && \
	cd ..

optipng/src/libpng/pnglibconf.h:
	cd optipng/src/libpng/ && \
	cp scripts/pnglibconf.h.prebuilt pnglibconf.h && \
	cd ../../..

# fixme: -fPIC
libhii.so: libhii.o
	$(LD) $(LDFLAGS) -shared -fPIC -o libhii.so libhii.o $(LIBDIR) $(LIBS)

libhii.dll: libhii.o
	$(LD) $(LDFLAGS) -shared -fPIC -o libhii.dll libhii.o $(LIBDIR) $(LIBS)

clean:
	-rm *.o *.a hii
	cd optipng && \
	$(MAKE) clean && \
	cd ..

install:
	@echo use the ./hii binary.

.PHONY: all clean install
