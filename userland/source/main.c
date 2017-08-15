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
	
	#ifndef NAME
	#define NAME "Super Cool Homebrew"
	#endif
	#define path_txt "/A9SP/path.txt"
	#define path_splash "romfs:/splash.bin"
	#define path_payloadrfs "romfs:/payload.firm"
	
	mkdir("/A9SP", 0777);
	mkdir("/A9SP/payloads", 0777);
	
	u8* contents;
	char path_payloadsd[255];

	snprintf(path_payloadsd, 21+sizeof(NAME), "/A9SP/payloads/%s.firm", NAME);
	
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
	
	
	FILE *payloadsd = fopen(path_payloadsd, "r");
	if(payloadsd == NULL)
	{
		printf("Copying payload to %s", path_payloadsd);
		payloadsd = fopen(path_payloadsd, "w");
		FILE *payloadrfs = fopen(path_payloadrfs, "r");
		fseek(payloadrfs, 0, SEEK_END);
		size_t payloadrfs_size = ftell(payloadrfs);
		rewind(payloadrfs);
		contents = malloc(payloadrfs_size);
		fread(contents, 1, payloadrfs_size, payloadrfs);
		fwrite(contents, 1, payloadrfs_size, payloadsd);
		free(contents);
		fclose(payloadrfs);
		fclose(payloadsd);
	}
	
	FILE *txt = fopen(path_txt, "w");
	fprintf(txt, path_payloadsd);
	fclose(txt);
	APT_HardwareResetAsync();
	
    gfxExit();
    return 0;
}
