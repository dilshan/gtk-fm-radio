TARGET=gtk-fm-tuner

CC=gcc
DEBUG=-g
OPT=-O0
WARN=-Wall

PTHREAD=-pthread

CCFLAGS=$(DEBUG) $(OPT) $(WARN) $(PTHREAD) -pipe

GTKLIB=`pkg-config --cflags --libs gtk+-3.0`

LD=gcc
LDFLAGS=$(PTHREAD) $(GTKLIB) -l wiringPi -export-dynamic

OBJS=resources.o qn8035.o freqedit.o main.o

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

main.o: src/main.c
	$(CC) -c $(CCFLAGS) src/main.c $(GTKLIB) -o main.o

freqedit.o: src/freqedit.c
	$(CC) -c $(CCFLAGS) src/freqedit.c $(GTKLIB) -o freqedit.o

qn8035.o: src/qn8035.c
	$(CC) -l wiringPi -c $(CCFLAGS) src/qn8035.c $(GTKLIB) -o qn8035.o

resources.o: src/resources.c
	$(CC) -c $(CCFLAGS) src/resources.c $(GTKLIB) -o resources.o

clean:
	rm -f *.o $(TARGET)

updateres:
	cd src; glib-compile-resources gtkfmtuner.gresource.xml --generate-source --target=resources.c