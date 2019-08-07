CRUFT = *.elf *.hex *.lst *.o *.as
LIBS = serial.o logger.o

all: clean controller upload

controller: libraries
	avr-gcc -std=c99 -Os -DF_CPU=16000000UL -mmcu=atmega328p -c -o controller.o controller.c
	avr-gcc -std=c99 -mmcu=atmega328p -o controller.elf controller.o $(LIBS)
	avr-objcopy -O ihex -R .eeprom controller.elf controller.hex

libraries:
	avr-gcc -std=c99 -Os -DF_CPU=16000000UL -mmcu=atmega328p -c -o serial.o lib/serial.c 
	avr-gcc -std=c99 -Os -DF_CPU=16000000UL -mmcu=atmega328p -c -o logger.o lib/logger.c 

assembly:
	avr-gcc -std=c99 -Os -DF_CPU=16000000UL -mmcu=atmega328p -S -o controller.as controller.c

upload:
	sudo avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyACM0 -b 115200 -U flash:w:controller.hex

clean:
	rm -f $(CRUFT)
