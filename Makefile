# fixme: fail to build with make -j4

# cross build
COMPILER_PREFIX=
#COMPILER_PREFIX=i686-w64-mingw32-

# do not enable optpng
#no_optpng=1

#CC=gcc
CC=$(COMPILER_PREFIX)gcc

CFLAGS=-O2 -std=c99 -Wall
#CFLAGS += -DNDEBUG
ifdef no_optpng
    CFLAGS += -D_NO_OPTPNG=1
endif

#LD=g++
LD=$(COMPILER_PREFIX)g++
LDFLAGS=-O2 -Wall
AR=$(COMPILER_PREFIX)ar
RANLIB=$(COMPILER_PREFIX)ranlib

# is ar, ranlib platform specific? yes.
My_make_flags=CC=$(CC) LD=$(LD) AR=$(AR) RANLIB=$(RANLIB)

ifdef no_optpng
    OPNG_LIB=
    OPNG_LIBDIR=
    OPNG_LINK=
else
    OPNG_LIB=optipng/src/pngxtern/libpngxtern.a \
      optipng/src/minitiff/libminitiff.a \
      optipng/src/pnmio/libpnmio.a \
      optipng/src/gifread/libgifread.a \
      optipng/src/libpng/libpng.a \
      optipng/src/zlib/libz.a \
      optipng/src/opngreduc/libopngreduc.a \
      optipng/src/opnglib/libopng.a
    OPNG_LIBDIR=-L optipng/src/opnglib/ -L optipng/src/libpng/ -L optipng/src/zlib/ -L optipng/src/pngxtern/ -L optipng/src/minitiff/ -L optipng/src/gifread/ -L optipng/src/pnmio/ -L optipng/src/opngreduc/
    OPNG_LINK=-lopng -lpng -lz -lpngxtern -lminitiff -lgifread -lpnmio -lopngreduc
endif


all: hii

hii: main.o libhii.a $(OPNG_LIB)
	$(LD) $(LDFLAGS) -static-libgcc -s main.o -o hii -L. -lhii $(OPNG_LIBDIR) $(OPNG_LINK)

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

clean:
	-rm *.o *.a hii
	cd optipng && \
	$(MAKE) clean && \
	cd ..

install:
	@echo use the ./hii binary.

update-stb:
	wget -O stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
	wget -O stb_image_write.h https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

.PHONY: all clean install update-stb
