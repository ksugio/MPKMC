CC = gcc
CFLAGS = -O2
INCLUDES = -I../lib -I/usr/include/python2.6
LIBS = -L../lib -lMPKMC -lm
TARGET = test
SOURCE = main.c

$(TARGET):	$(SOURCE)	
		$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(SOURCE) $(LIBS)

clean:
		rm $(TARGET)

