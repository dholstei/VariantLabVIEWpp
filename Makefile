##############################################################################
#
#                     Makefile for LvVariant.so
#
##############################################################################

# This variable contains the flags passed to cc.

CC = gcc
C++ = g++
CFLAGS = -g -m32 -fPIC -Di686
CXXFLAGS = -g -m32 -std=gnu++17 -lstdc++

# LabVIEW
INCLUDES := $(INCLUDES)  -I/usr/local/lv71/cintools
LIBS := $(LIBS) /usr/local/lv71/AppLibs/liblvrt.so.7.1

SDL_LIB=/usr/lib/libSDL2-2.0.so.0

#  DLLFLAGS = -shared -W1,-soname,$@
DLLFLAGS = -shared -m32
.SUFFIXES:
.SUFFIXES: .c .cpp .o .so

OBJECTS = LvVariant.$(OBJ)

.c:
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS)

.c.o: 
	$(CC) $(INCLUDES) -c $(CFLAGS) $<

.cpp.o: 
	$(C++) $(INCLUDES) -c $(CXXFLAGS) $<

.c.obj: 
	$(CC) $(INCLUDES) -c $(CFLAGS) $<

all:    LvVariant++.so

LvVariant++:	LvVariant++.o
	$(C++) $(CXXFLAGS) -o $@ $? $(MYSQLCPP_LIB) -ldl -lpthread -lresolv -lssl -lcrypto

LvVariant++.so: LvVariant++.cpp
	$(C++) $(CXXFLAGS) -shared -fPIC -o $@ $? $(INCLUDES) $(LIBS)\
	 -ldl -lpthread -lresolv -lssl -lcrypto

LvVariant.so:    $(OBJECTS)
	$(CC) $(DLLFLAGS) -o $@ $(OBJECTS) $(LIBS)

LvVariant.dll:    $(OBJECTS)
	$(LINKER) $(DLLFLAGS) $(OBJECTS) $(LIBS)

clean:
	 rm -f\
	 LvVariant.$(SUFFIX) *.$(OBJ) LvVariant++

dist:
	 tar cvfz LvVariant.tgz *.c *.h Makefile *.llb

test:    LvVariant++.so
	 $(CC) $(CXXFLAGS) test.cpp -o test $<

