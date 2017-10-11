#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <3ds.h>

int main() {
    romfsInit();
	
	gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);
	
	#define path_splash "romfs:/splash.bin"
	#define path_payloadrfs "romfs:/payload.firm"
	#define path_chainloader "romfs:/A9SP.firm"
	#define path_chainloader_sha "romfs:/A9SP.firm.sha"
	#define path_default "boot0.firm"
	#define path_bootfirm "boot.firm"
	#define FIRM_MAXSIZE (0x3FFB00)
	
	u8* contents;
	
	
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
	
	u8 *buf = (u8*) linearMemAlign(FIRM_MAXSIZE + 0x200, 0x400000);
	
	FILE *payload = fopen(path_payloadrfs, "r");
	fseek(payload, 0, SEEK_END);
	size_t payload_size = ftell(payload);
	rewind(payload);
	if(fread(buf + 0x200, 1, payload_size, payload) != payload_size) printf("Failed copy to RAM");
	memcpy(buf, "A9NC", 4);
	fclose(payload);
	
	for(int i=0; i<=120; i++)	for(int i=0; i<=120; i++);
	APT_HardwareResetAsync();
	
    gfxExit();
    return 0;
}
