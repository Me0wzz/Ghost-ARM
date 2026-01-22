CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -g -mcpu=arm926ej-s -ffreestanding -Wall -Wextra -Iinclude -c
LDFLAGS = -T boot/linker.ld

SRC_DIRS = boot kernel drivers lib apps
C_SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
ASM_SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))

OBJS = $(addprefix build/, $(notdir $(C_SRCS:.c=.o))) \
	$(addprefix build/, $(notdir $(ASM_SRCS:.s=.o)))

$(OBJS): | build_dir

TARGET = ghost-arm

all: build_dir $(TARGET).axf

build_dir:
	@mkdir -p build

$(TARGET).axf: build/$(TARGET).axf
	cp $< $@
	@echo ">>> ELF File Generated: $@"

build/$(TARGET).axf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

build/%.o: kernel/%.c
	$(CC) $(CFLAGS) -o $@ $<
build/%.o: drivers/%.c
	$(CC) $(CFLAGS) -o $@ $<
build/%.o: lib/%.c
	$(CC) $(CFLAGS) -o $@ $<
build/%.o: apps/%.c
	$(CC) $(CFLAGS) -o $@ $<
build/%.o: boot/%.s
	$(CC) $(CFLAGS) -o $@ $<

 

clean:
	rm -rf build $(TARGET).axf


run: $(TARGET).axf
	qemu-system-arm -M versatilepb -m 128M -kernel $(TARGET).axf -drive if=pflash,format=raw,file=flash.bin -nographic