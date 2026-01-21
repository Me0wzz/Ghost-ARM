CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -g -mcpu=arm926ej-s -ffreestanding -Wall -Wextra
LDFLAGS = -T linker.ld

OBJS = startup.o kernel.o shell.o lib.o mm.o console.o tetris.o flash.o main.o
TARGET = ghost-arm

all: $(TARGET).bin

$(TARGET).bin: $(TARGET).axf
	$(OBJCOPY) -O binary $< $@

$(TARGET).axf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.s
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.axf *.bin

run: ghost-arm.axf
	qemu-system-arm -M versatilepb -m 128M -kernel ghost-arm.axf -drive if=pflash,format=raw,file=flash.bin -nographic