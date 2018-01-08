#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <3ds.h>

#include "LED.h"
#include "MCU.h"
	
#define path_splash "romfs:/splash.bin"
#define path_payloadrfs "romfs:/payload.firm"
#define path_chainloader "romfs:/A9SP.firm"
#define path_chainloader_sha "romfs:/A9SP.firm.sha"
#define path_default "boot0.firm"
#define path_bootfirm "boot.firm"
#define FIRM_MAXSIZE (0x3FFB00)
#define FASTBOOT_SLOT (u8)0x06

u8 GetSlot_FastBoot3DS(u8 *out)
{	
	mcuInit();
	mcuReadRegister(0x1E, out, 0x01);
	mcuExit();
	return (u32)out;
}

u8 SetSlot_FastBoot3DS(u8 slot)
{
	mcuInit();
	mcuWriteRegister(0x1E, &slot, 0x01);
	mcuExit();
	return slot;
}

int main() {
    romfsInit();
	
	gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);
	
	u8* contents;
	u8  slot = 0;
	
    FILE *splash = fopen(path_splash, "rb");
	fseek(splash, 0, SEEK_END);
	size_t splash_size = ftell(splash);
	rewind(splash);
	contents = malloc(splash_size);
	fread(contents, 1, splash_size, splash);
	memcpy(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), contents, splash_size);
	free(contents);
	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();
	
	if((!fopen(path_default, "r")) && (GetSlot_FastBoot3DS(&slot) == 0))
	{
		printf("%s not found on the root of the SD", path_default);
		gfxFlushBuffers();
		gfxSwapBuffers();
		fixcolor(255,0,0);
		for(int i=0; i<=300; i++) gspWaitForVBlank();
		errfInit();
		ERRF_ThrowResultWithMessage((Result) 'A9SP', "Read the README next time");
		errfExit();
	}
	u8 *buf = (u8*) linearMemAlign(FIRM_MAXSIZE + 0x200, 0x400000);
	memcpy(buf, "A9NC", 4);
	
	FILE *payload = fopen(path_payloadrfs, "r");
	fseek(payload, 0, SEEK_END);
	size_t payload_size = ftell(payload);
	rewind(payload);
	if(fread(buf + 0x200, 1, payload_size, payload) != payload_size)
		fixcolor(255, 0, 0);
	else
		fixcolor(0, 255, 0);
	
	fclose(payload);
	
	if((GetSlot_FastBoot3DS(&slot) > 0) && (slot != FASTBOOT_SLOT))
	{
		//printf("FastBoot3DS slot found, using slot %u, changing to slot %u", slot, FASTBOOT_SLOT);
		SetSlot_FastBoot3DS(FASTBOOT_SLOT);
	}
	else
	{
		//printf("Slot change not needed");
	}
	for(int i=0; i<=60; i++) gspWaitForVBlank();
	stfuled();
	APT_HardwareResetAsync();
	
    gfxExit();
    return 0;
}
