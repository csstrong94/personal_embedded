# makefile for blink.c LED application
# also flashes FRAM
# chris strong


# dirs
LINKER_DIR = /usr/bin/msp430-gcc/include/
BUILD_DIR = build
OBJECT_DIR = build/obj
SRC_DIR = src

INCLUDES = \
	   include/ \
	   

SOURCES = $(wildcard $(SRC_DIR)/*.c)

PLATFORM = msp430fr6989
CC = msp430-elf-gcc
BASE = cb
TARGET = $(BASE).elf

#flasher vars
FLASHER = MSP430Flasher
FLASH_TARGET = $(BASE).hex
VCC = [VCC]
FFLAGS = -n $(PLATFORM) -v -g -z $(VCC) -w


#preprocessor flags
CPPFLAGS = -I$(INCLUDES) -D__MSP430FR6989__

#compiler flags
CFLAGS = -Wall -g -O0 -std=c99 

#linker flags
LINKER_FILE = msp430fr6989.ld
LDFLAGS = -Wl,-Map=$(BASE).map,--unresolved-symbols=ignore-in-object-files \
	  -mmcu=$(PLATFORM) \
	   -msmall -T $(LINKER_FILE)
# objcopy flags
OBJCOPY = msp430-elf-objcopy
OCFLAGS = -O $(EXEC_FORMAT) $(TARGET) $(BASE).hex
EXEC_FORMAT = ihex


OBJS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJECT_DIR)/%.o)
$(info $(SOURCES))

%.i : $(SRC_DIR)/%.c

	$(CC) $(CPPFLAGS) -E $(CLFAGS) $^ -o $@


$(OBJECT_DIR)/%.o : $(SRC_DIR)/%.c
	@echo compiling $< 
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $(OBJECT_DIR)/$(@F)

.PHONY: build write-fram clean

build : $(TARGET)

$(TARGET) : $(OBJS) 
	$(CC) $(LDFLAGS) $^  -o $@
	$(OBJCOPY) $(OCFLAGS)
	
.PHONY: write-fram

write-fram : $(FLASH_TARGET)
	$(FLASHER) $(FFLAGS) "$(FLASH_TARGET)" 

.PHONY: clean

clean : 
	$(info *** cleaning ***)
	rm -rf *.o *.elf *.hex *.bin *.map *.i *.d
	cd build/obj; rm -rf *





