#ifndef UEFI_H
#define UEFI_H

#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
);

#endif // UEFI_H