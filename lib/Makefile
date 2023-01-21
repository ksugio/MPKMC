CC = gcc
CFLAGS = -O2 -fPIC -DMP_PYTHON_LIB
INCLUDES = -I/usr/include/python3.8 -I/usr/local/lib/python3.8/dist-packages/numpy/core/include
LIBS = -lz
TARGET_D = MPKMC.so
TARGET_S = libMPKMC.a
INSTALL_DIR = ../python
OBJS = event.o fsfcc.o jump.o kmc.o meam.o python.o rand.o rotindex.o rw.o solute.o types.o

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

event.c:    MPKMC.h
fsfcc.c:	MPKMC.h
jump.c:     MPKMC.h
kmc.c:		MPKMC.h
meam.c:     MPKMC.h
python.c:	MPKMC.h
rand.c:		MPKMC.h
rotindex.c:	MPKMC.h
rw.c:       MPKMC.h
solute.c:   MPKMC.h
types.c:    MPKMC.h
