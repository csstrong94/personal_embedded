# makefile for blink.c LED application
# also flashes FRAM
# chris strong

SOURCES = blink.c
INCLUDES = /usr/bin/msp430-gcc/include/

PLATFORM = msp430fr6989
CC = msp430-elf-gcc
BASE = blink
TARGET = $(BASE).elf

#flasher vars
FLASHER = MSP430Flasher
FLASH_TARGET = $(BASE).hex
VCC = [VCC]
FFLAGS = -n $(PLATFORM) -v -g -z $(VCC)


EXEC_FORMAT = ihex

OBJCOPY = msp430-elf-objcopy
OCFLAGS = -O $(EXEC_FORMAT) $(TARGET) $(BASE).hex

CFLAGS = -Wall -g -O0 -std=c99 -mmcu=$(PLATFORM) -msmall \
	 -I$(INCLUDES)
LDFLAGS = -Wl,-Map=$(BASE).map -L $(INCLUDES) 

OBJS = $(SOURCES:.c=.o)


%.o : %.c 
	$(CC) $(CFLAGS) -c $< -O $@

.PHONY: build

build : $(TARGET)

$(TARGET) : $(SOURCES) 
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	$(OBJCOPY) $(OCFLAGS)
	
.PHONY: write-fram

write-fram : $(FLASH_TARGET)
	$(FLASHER) -w "$(FLASH_TARGET)" $(FFLAGS)

.PHONY: clean

clean : 
	$(info *** cleaning ***)
	rm -f *.o *.elf *.hex *.bin *.map





