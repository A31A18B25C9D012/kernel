AS = nasm
CC = gcc
LD = ld

ASFLAGS = -f bin
CFLAGS = -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector -mno-red-zone -O2 -fno-builtin -Iinclude
LDFLAGS = -m elf_i386 -T src/kernel/linker.ld

KERNEL_OBJS = build/cache.o build/xfce.o build/memory.o build/framebuffer.o build/font.o build/gui.o build/input.o build/main.o

all: os.img

os.img: build/boot.bin build/kernel.bin
	cat build/boot.bin > os.img
	dd if=build/kernel.bin of=os.img bs=512 seek=1 conv=notrunc 2>/dev/null
	truncate -s 33M os.img

build/boot.bin: boot/boot.asm
	$(AS) $(ASFLAGS) boot/boot.asm -o build/boot.bin

build/kernel.bin: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $(KERNEL_OBJS) -o build/kernel.elf
	objcopy -O binary build/kernel.elf build/kernel.bin
	@SIZE=$$(stat -c%s build/kernel.bin); \
	echo "kernel size: $$SIZE bytes"; \
	if [ $$SIZE -gt 262144 ]; then \
		echo "warning: kernel exceeds 256KB L2 cache target"; \
	fi

build/cache.o: src/kernel/cache.c include/types.h
	$(CC) $(CFLAGS) -c $< -o $@

build/xfce.o: src/kernel/xfce.c include/types.h
	$(CC) $(CFLAGS) -c $< -o $@

build/memory.o: src/kernel/memory.c include/types.h
	$(CC) $(CFLAGS) -c $< -o $@

build/framebuffer.o: src/kernel/framebuffer.c include/types.h
	$(CC) $(CFLAGS) -c $< -o $@

build/font.o: src/kernel/font.c include/types.h
	$(CC) $(CFLAGS) -c $< -o $@

build/gui.o: src/kernel/gui.c include/types.h
	$(CC) $(CFLAGS) -c $< -o $@

build/input.o: src/kernel/input.c include/types.h
	$(CC) $(CFLAGS) -c $< -o $@

build/main.o: src/kernel/main.c include/types.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f build/* os.img

run: os.img
	qemu-system-i386 -drive format=raw,file=os.img -m 512M


.PHONY: all clean run
