
CC = x86_64-elf-gcc
CFLAGS = -I./src/include -fPIC -fno-stack-protector -mno-red-zone -m64 -fshort-enums -fno-PIE -fno-asynchronous-unwind-tables
LDFLAGS = -nostdlib -Wl,--no-dynamic-linker -Wl,--entry=efi_main -Wl,--image-base=0x100000

SRC = src/main.c src/kernel.c src/graphics.c src/task.c
OBJ = $(SRC:.c=.o)

EFI_TARGET = HACOS.efi
ISO_TARGET = hacos.iso

all: $(EFI_TARGET) $(ISO_TARGET)

$(EFI_TARGET): $(OBJ)
	$(CC) $(CFLAGS) -DEFI_FUNCTION_WRAPPER -c src/main.c -o src/main.o
	$(CC) $(CFLAGS) -c src/kernel.c -o src/kernel.o
	$(CC) $(CFLAGS) -c src/graphics.c -o src/graphics.o
	$(CC) $(CFLAGS) -c src/task.c -o src/task.o
	$(CC) $(CFLAGS) -nostdlib -Wl,--no-dynamic-linker -Wl,--entry=efi_main -Wl,--image-base=0x100000 -shared -o $(EFI_TARGET) src/main.o src/kernel.o src/graphics.o src/task.o

$(ISO_TARGET): $(EFI_TARGET)
	mkdir -p isodir/EFI/BOOT
	cp $(EFI_TARGET) isodir/EFI/BOOT/BOOTX64.EFI
	xorriso -as mkisofs -R -f -e EFI/BOOT/BOOTX64.EFI -no-emul-boot -o $(ISO_TARGET) isodir

run: $(ISO_TARGET)
	qemu-system-x86_64 -bios /usr/share/OVMF/OVMF.fd -cdrom $(ISO_TARGET)

clean:
	rm -f src/*.o $(EFI_TARGET) $(ISO_TARGET)
	rm -rf isodir

.PHONY: all clean run