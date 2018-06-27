CC = gcc
CFLAGS = -O2
INCLUDES = -I../lib -I../libgl -I/usr/include/python2.6
LIBS = -L../lib -lMPKMC -lm -lz -L../libgl -lMPGLKMC -lGL -lGLU -lglut -lpython2.6
TARGET = test
SOURCE = main.c glut.c

$(TARGET):	$(SOURCE)	
		$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(SOURCE) $(LIBS)

clean:
		rm $(TARGET)

