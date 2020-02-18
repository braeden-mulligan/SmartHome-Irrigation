CC := avr-gcc
CFLAGS := -std=c99 -I include -Os -DF_CPU=16000000UL -mmcu=atmega328p -c 
LFLAGS := -std=c99 -mmcu=atmega328p

MAIN := main.c
#SRC := $(wildcard ./src/*.c)
#SRC := $(shell find ./src -name *.c)
OBJS := main.o logger.o hardware.o
LIBS := uart.o timer.o
TARGET := irrigation_controller

CRUFT = *.elf *.hex *.lst *.o *.as

all: clean target upload

target: sources libraries 
	@echo "Building $(TARGET)..."
	$(CC) $(LFLAGS) -o $(TARGET).elf $(OBJS) $(LIBS)
	avr-objcopy -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex

sources:
	@echo "Making object files..." 
	$(CC) $(CFLAGS) -o main.o ./src/main.c
	$(CC) $(CFLAGS) -o logger.o ./src/logger.c
	$(CC) $(CFLAGS) -o hardware.o ./src/hardware.c

libraries:
	@echo "Making libraries..." 
	$(CC) $(CFLAGS) -o uart.o lib/uart.c
	$(CC) $(CFLAGS) -o timer.o lib/timer.c

assembly:
	avr-gcc -std=c99 -Os -DF_CPU=16000000UL -mmcu=atmega328p -S -o $(TARGET).as $(TARGET).c

upload:
	sudo avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyACM0 -b 115200 -U flash:w:$(TARGET).hex

.PHONY: clean
clean:
	@echo "Cleaning..."
	rm -f $(CRUFT)
