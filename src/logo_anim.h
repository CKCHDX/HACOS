// HACOS Boot Logo Animation Header
#ifndef HACOS_LOGO_ANIM_H
#define HACOS_LOGO_ANIM_H

#include <efi.h>
#include <efilib.h>

void logo_anim_play(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop, int x, int y);

#endif // HACOS_LOGO_ANIM_H
