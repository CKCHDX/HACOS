
#include <efi.h>
#include <efilib.h>
#include "kernel.c"

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_STATUS status = SystemTable->BootServices->LocateProtocol(
        &gEfiGraphicsOutputProtocolGuid, NULL, (void **)&gop);
    if (EFI_ERROR(status)) {
        Print(L"Unable to locate GOP\n");
        return status;
    }

    // Call kernel main, which will initialize everything and draw the logo
    hacos_kernel_main(SystemTable, gop);

    // Wait for key press before exit
    EFI_INPUT_KEY key;
    SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key);

    return EFI_SUCCESS;
}