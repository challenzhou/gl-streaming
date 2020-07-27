CFLAGS= -std=c++11 -g3 -Wall -DGLS_CLIENT

INCLUDES=-I..  -I../common
LIBS=-lpthread -lm -ldl -lX11 -lEGL -lGLESv2
OBJS= egl_test.o glsutil.o 
EXES=egl_test 

all: $(EXES)


glsutil.o: ../glsutil.c ../glsutil.h
	gcc -c -o glsutil.o $(CFLAGS) $(INCLUDES) ../glsutil.c

egl_test.o: egl_test.c ../glsutil.h
	gcc -c -o egl_test.o $(CFLAGS) $(INCLUDES) egl_test.c

egl_test: egl_test.o glsutil.o 
	gcc -o egl_test $(CFLAGS) $(INCLUDES) $(OBJS) $(LIBS)

clean:
	@rm -fr $(OBJS) egl_test
