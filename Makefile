# HTTP-JSON-Assistant Makefile
# Based on Serial-JSON-Bridge pattern

# Target firmware name
TARGET = http_json_assistant

# Toolchain
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size

# MCU settings
CPU = -mcpu=cortex-m0plus
MCU = $(CPU) -mthumb

# Source files (will expand as we add drivers)
SRCS = src/main.c \
       src/startup.c \
       src/syscalls.c \
       src/drivers/rcc.c \
       src/drivers/gpio.c \
       src/drivers/spi.c \
       src/drivers/enc28j60.c \
       src/drivers/delay.c

# Include directories
INCLUDES = -Isrc -Isrc/drivers

# Derived files
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

# Compiler flags
CFLAGS = -Wall -Wextra -g -std=c11 -Os
CFLAGS += $(MCU)
CFLAGS += $(INCLUDES)
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -MMD -MP  # Generate dependency files

# Linker flags
LDFLAGS = -T linker/STM32G071RB.ld
LDFLAGS += $(MCU)
LDFLAGS += -Wl,-Map=$(TARGET).map
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -nostartfiles
LDFLAGS += --specs=nosys.specs

# Default target
all: $(TARGET).elf $(TARGET).bin $(TARGET).hex size

# Link object files to ELF
$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Generate binary file
$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Generate hex file
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

# Compile C files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Show size information
size: $(TARGET).elf
	$(SIZE) $<

# Generate disassembly
disasm: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $(TARGET).asm

# Flash to target (requires OpenOCD and ST-Link)
flash: $(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32g0x.cfg \
	        -c "program $(TARGET).elf verify reset exit"

# Clean build artifacts
clean:
	rm -f $(OBJS) $(DEPS)
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).map $(TARGET).asm
	rm -f src/*.o src/*.d
	rm -f src/drivers/*.o src/drivers/*.d
	rm -f tests/unit/*.o tests/unit/*.d

# Include dependency files
-include $(DEPS)

# Unit test targets
test-gpio:
	$(CC) $(CFLAGS) -o test_gpio.elf tests/unit/test_gpio.c \
	      src/drivers/rcc.c src/drivers/gpio.c src/drivers/spi.c src/drivers/delay.c \
	      src/startup.c src/syscalls.c $(LDFLAGS)
	$(OBJCOPY) -O binary test_gpio.elf test_gpio.bin
	$(SIZE) test_gpio.elf

test-delay:
	$(CC) $(CFLAGS) -o test_delay.elf tests/unit/test_delay.c \
	      src/drivers/rcc.c src/drivers/gpio.c src/drivers/spi.c src/drivers/delay.c \
	      src/startup.c src/syscalls.c $(LDFLAGS)
	$(OBJCOPY) -O binary test_delay.elf test_delay.bin
	$(SIZE) test_delay.elf

test-spi:
	$(CC) $(CFLAGS) -o test_spi.elf tests/unit/test_spi.c \
	      src/drivers/rcc.c src/drivers/gpio.c src/drivers/spi.c src/drivers/delay.c \
	      src/startup.c src/syscalls.c $(LDFLAGS)
	$(OBJCOPY) -O binary test_spi.elf test_spi.bin
	$(SIZE) test_spi.elf

# Flash test targets
flash-test-gpio: test-gpio
	openocd -f interface/stlink.cfg -f target/stm32g0x.cfg \
	        -c "program test_gpio.elf verify reset exit"

flash-test-delay: test-delay
	openocd -f interface/stlink.cfg -f target/stm32g0x.cfg \
	        -c "program test_delay.elf verify reset exit"

flash-test-spi: test-spi
	openocd -f interface/stlink.cfg -f target/stm32g0x.cfg \
	        -c "program test_spi.elf verify reset exit"

# Clean test artifacts
clean-tests:
	rm -f test_*.elf test_*.bin test_*.map

.PHONY: all clean flash disasm size test-gpio test-delay test-spi flash-test-gpio flash-test-delay flash-test-spi clean-tests
