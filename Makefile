CC := avr-gcc
CFLAGS := -std=gnu99 -I include -I lib -Os -DF_CPU=16000000UL -mmcu=atmega328p -Wall 
LFLAGS := -std=gnu99 -mmcu=atmega328p -Wall

SRC := $(shell ls src/*.c)
TMP := $(subst src/,build/,$(SRC))
OBJ := $(subst .c,.o,$(TMP))

LIB_DIR := lib
LIB_SRC := $(shell ls lib/*.c)
LIB_TMP := $(subst $(LIB_DIR)/,build/,$(LIB_SRC))
OBJ += $(subst .c,.o,$(LIB_TMP)) 

TARGET := smarthome-irrigation

CRUFT := *.o *.elf

all: target

full: clean target 

uno: target | upload_uno

nano: target | upload_nano

target: sources
	@mkdir -p bin
	@echo "Building $(TARGET)..."
	$(CC) $(LFLAGS) -o ./build/$(TARGET).elf $(OBJ)
	avr-objcopy -O ihex -R .eeprom ./build/$(TARGET).elf bin/$(TARGET).hex

sources: $(OBJ) 
build/%.o: src/%.c | build_dir
	@echo "Compiling source code..."
	$(CC) $(CFLAGS) -o $@ -c $<

build/%.o: $(LIB_DIR)/%.c | build_dir
	@echo "Compiling libraries..."
	$(CC) $(CFLAGS) -o $@ -c $<


upload_uno: bin/$(TARGET).hex | bin
	sudo avrdude -F -V -c arduino -p m328p -P /dev/ttyACM0 -b 115200 -U flash:w:./bin/$(TARGET).hex:i

upload_nano: bin/$(TARGET).hex | bin
	sudo avrdude -F -V -c arduino -p m328p -P /dev/ttyUSB0 -b 57600 -U flash:w:./bin/$(TARGET).hex:i

.PHONY: build_dir
build_dir:
	@mkdir -p build

.PHONY: clean
clean: 
	rm -f ./bin/*
	@echo "Cleaning..."
	rm -f ./build/$(CRUFT)
