CC = gcc
CFLAGS = -O2 -fPIC -DMP_PYTHON_LIB
INCLUDES = -I/usr/include/python2.6
LIBS = -lz
TARGET_D = MPKMC.so
TARGET_S = libMPKMC.a
INSTALL_DIR = ../python
OBJS = kmc.o rand.o python.o rotindex.o jump.o rw.o types.o fsfcc.o

all:	$(TARGET_D) $(TARGET_S)

$(TARGET_D):	$(OBJS)
		$(CC) -shared -o $@ $(OBJS) $(LIBS)

$(TARGET_S):	$(OBJS)
		ar r $@ $(OBJS)

install:	$(TARGET_D)
		cp $(TARGET_D) $(INSTALL_DIR)

clean:
		rm $(TARGET_D) $(TARGET_S) $(OBJS)

.c.o:
		$(CC) $(CFLAGS) $(INCLUDES) -c $<

kmc.c:		MPKMC.h
rand.c:		MPKMC.h
python.c:	MPKMC.h
rotindex.c:	MPKMC.h
fsfcc.c:	MPKMC.h

