CC = gcc
CXX = g++
LD = ld
OBJCOPY = objcopy

# Compiler flags for UEFI bootloader
CFLAGS = -Wall -Wextra -O2 -ffreestanding -fno-stack-protector -fpic \
         -mno-red-zone \
         -I/usr/include/efi -I/usr/include/efi/x86_64 -Iinclude \
         -DEFI_FUNCTION_WRAPPER \
         -fshort-wchar

LDFLAGS = -nostdlib -znocombreloc -T /usr/lib/elf_x86_64_efi.lds \
          -shared -Bsymbolic -L/usr/lib /usr/lib/crt0-efi-x86_64.o

LIBS = -lefi -lgnuefi

# C++ flags for userspace
CXXFLAGS = -Wall -Wextra -O2 -ffreestanding -fno-exceptions -fno-rtti \
           -fno-stack-protector -mno-red-zone -Iinclude -mcmodel=large \
           -mno-mmx -mno-sse -mno-sse2 -std=c++17

# Kernel C flags
KERNEL_CFLAGS = -Wall -Wextra -O2 -ffreestanding -fno-stack-protector \
                -mno-red-zone -Iinclude -mcmodel=large \
                -mno-mmx -mno-sse -mno-sse2

BUILD_DIR = build
BOOT_DIR = boot
KERNEL_DIR = kernel
DRIVERS_DIR = drivers
UI_DIR = ui
USERSPACE_DIR = userspace

BOOT_OBJS = $(BUILD_DIR)/uefi_main.o $(BUILD_DIR)/boot_ui.o
KERNEL_OBJS = $(BUILD_DIR)/kernel.o $(BUILD_DIR)/graphics.o

# Userspace objects - WITH login subsystem
USERSPACE_OBJS = $(BUILD_DIR)/main.o \
                 $(BUILD_DIR)/renderer.o \
                 $(BUILD_DIR)/input_manager.o \
                 $(BUILD_DIR)/font_renderer.o \
                 $(BUILD_DIR)/gfx_effects.o \
                 $(BUILD_DIR)/memory.o \
                 $(BUILD_DIR)/desktop.o \
                 $(BUILD_DIR)/mouse_manager.o \
                 $(BUILD_DIR)/login_consumer.o

all: $(BUILD_DIR)/BOOTX64.EFI $(BUILD_DIR)/kernel.bin

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# UEFI bootloader
$(BUILD_DIR)/uefi_main.o: $(BOOT_DIR)/uefi_main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/boot_ui.o: $(UI_DIR)/boot_ui.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/BOOTX64.so: $(BOOT_OBJS) $(KERNEL_OBJS) $(USERSPACE_OBJS)
	$(LD) $(LDFLAGS) $(BOOT_OBJS) $(KERNEL_OBJS) $(USERSPACE_OBJS) -o $@ $(LIBS)

$(BUILD_DIR)/BOOTX64.EFI: $(BUILD_DIR)/BOOTX64.so
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym \
	           -j .rel -j .rela -j .reloc \
	           --target=efi-app-x86_64 $< $@

# Kernel
$(BUILD_DIR)/kernel.o: $(KERNEL_DIR)/kernel.c | $(BUILD_DIR)
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

$(BUILD_DIR)/graphics.o: $(DRIVERS_DIR)/graphics.c | $(BUILD_DIR)
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

# Userspace core
$(BUILD_DIR)/main.o: $(USERSPACE_DIR)/main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/renderer.o: $(USERSPACE_DIR)/renderer.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/input_manager.o: $(USERSPACE_DIR)/input_manager.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/font_renderer.o: $(USERSPACE_DIR)/font_renderer.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/gfx_effects.o: $(USERSPACE_DIR)/gfx_effects.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/desktop.o: $(USERSPACE_DIR)/desktop.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Login subsystem
$(BUILD_DIR)/login_animations.o: $(USERSPACE_DIR)/login/animations.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/login_particles.o: $(USERSPACE_DIR)/login/particles.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/login_ui_elements.o: $(USERSPACE_DIR)/login/ui_elements.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/login_engine.o: $(USERSPACE_DIR)/login/login_engine.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(BUILD_DIR)/memory.o: $(USERSPACE_DIR)/memory.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
# Link kernel binary
$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJS) $(USERSPACE_OBJS)
	$(LD) -T kernel.ld $(KERNEL_OBJS) $(USERSPACE_OBJS) -o $@
$(BUILD_DIR)/login_screen.o: $(USERSPACE_DIR)/login_screen.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@,

$(BUILD_DIR)/mouse_manager.o: $(USERSPACE_DIR)/mouse_manager.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/login_consumer.o: $(USERSPACE_DIR)/login_consumer.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
clean:
	rm -rf $(BUILD_DIR)

image: all
	mkdir -p $(BUILD_DIR)/iso/EFI/BOOT
	cp $(BUILD_DIR)/BOOTX64.EFI $(BUILD_DIR)/iso/EFI/BOOT/
	dd if=/dev/zero of=$(BUILD_DIR)/hacos.img bs=1M count=64
	mkfs.vfat -F32 $(BUILD_DIR)/hacos.img
	mmd -i $(BUILD_DIR)/hacos.img ::/EFI
	mmd -i $(BUILD_DIR)/hacos.img ::/EFI/BOOT
	mcopy -i $(BUILD_DIR)/hacos.img $(BUILD_DIR)/BOOTX64.EFI ::/EFI/BOOT/

run: image
	qemu-system-x86_64 \
	    -bios /usr/share/ovmf/OVMF.fd \
	    -drive file=$(BUILD_DIR)/hacos.img,format=raw \
	    -m 256M

.PHONY: all clean image run
