
#
#	Makefile for project
#

DEBUG = -g
CC = qcc
LD = qcc


TARGET = -Vgcc_ntox86_64
#TARGET = -Vgcc_ntox86
#TARGET = -Vgcc_ntoarmv7le
#TARGET = -Vgcc_ntoaarch64le


CFLAGS += $(DEBUG) $(TARGET) -Wall
LDFLAGS+= $(DEBUG) $(TARGET)
BINS = main flight_controller display propeller sensor navigation
all: $(BINS)

clean:
	rm -f *.o $(BINS);
#	cd solutions; make clean


main.o: main.c main.h
flight_controller.o: flight_controller.c flight_controller.h navigation.h
display.o: display.c display.h
propeller.o: propeller.c propeller.h
sensor.o: sensor.c sensor.h
navigation.o: navigation.c navigation.h flight_controller.h
