#include <efi.h>
#include <efilib.h>
#include "boot_ui.h"

#define KERNEL_LOAD_ADDRESS 0x100000

typedef void (*kernel_entry_t)(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *);

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    
    InitializeLib(ImageHandle, SystemTable);
    
    // Initialize graphics
    status = uefi_call_wrapper(
        BS->LocateProtocol,
        3,
        &gop_guid,
        NULL,
        (void **)&gop
    );
    
    if (EFI_ERROR(status)) {
        Print(L"Failed to locate Graphics Output Protocol\n");
        return status;
    }
    
    // Initialize boot UI
    init_boot_ui(gop);
    
    // ========================================
    // PHASE 1: Boot Animation (5 seconds)
    // ========================================
    show_boot_animation(gop, 5000);
    
    // ========================================
    // TRANSITION: Boot → Login
    // ========================================
    transition_boot_to_login(gop);
    
    // ========================================
    // PHASE 2: Allocate memory for kernel
    // ========================================
    EFI_PHYSICAL_ADDRESS kernel_addr = KERNEL_LOAD_ADDRESS;
    UINTN pages = 1024;  // Increased for C++ code
    
    status = uefi_call_wrapper(
        BS->AllocatePages,
        4,
        AllocateAddress,
        EfiLoaderData,
        pages,
        &kernel_addr
    );
    
    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate memory for kernel\n");
        return status;
    }
    
    // Copy kernel code
    extern void kernel_main(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *);
    UINT8 *kernel_code = (UINT8 *)kernel_addr;
    UINT8 *kernel_func = (UINT8 *)kernel_main;
    
    // Copy more memory to include C++ code
    for (UINTN i = 0; i < 16384; i++) {
        kernel_code[i] = kernel_func[i];
    }
    
    // Get memory map
    UINTN map_key;
    UINTN map_size = 0;
    UINTN desc_size;
    UINT32 desc_version;
    
    status = uefi_call_wrapper(
        BS->GetMemoryMap,
        5,
        &map_size,
        NULL,
        &map_key,
        &desc_size,
        &desc_version
    );
    
    map_size += 2 * desc_size;
    EFI_MEMORY_DESCRIPTOR *memory_map;
    
    status = uefi_call_wrapper(
        BS->AllocatePool,
        3,
        EfiLoaderData,
        map_size,
        (void **)&memory_map
    );
    
    status = uefi_call_wrapper(
        BS->GetMemoryMap,
        5,
        &map_size,
        memory_map,
        &map_key,
        &desc_size,
        &desc_version
    );
    
    // ========================================
    // EXIT BOOT SERVICES
    // After this point, NO MORE UEFI FUNCTIONS!
    // ========================================
    status = uefi_call_wrapper(
        BS->ExitBootServices,
        2,
        ImageHandle,
        map_key
    );
    
    if (EFI_ERROR(status)) {
        Print(L"Failed to exit boot services\n");
        return status;
    }
    
    // ========================================
    // PHASE 3: Jump to kernel (C++ userspace)
    // ========================================
    kernel_entry_t kernel_entry = (kernel_entry_t)kernel_addr;
    kernel_entry(gop->Mode);
    
    while(1);
    return EFI_SUCCESS;
}
