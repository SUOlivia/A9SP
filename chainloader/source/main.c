#include "common.h"
#include "power.h"
#include "ff.h"
#include "firm.h"
#include "sha.h"
#include "i2c.h"

//for debug purposes only
#include "draw.h"

#define FIRM_BUFFER  ((void*)0x27C00000)
#define FIRM_MAXSIZE (0x3FFB00)

#define RANGE(x,a,b) (((x)>=(a))&&((x)<=(b)))

u32 wlist[][2] = {
	{0x08000000, 0x08100000},
	{0x18000000, 0x18600000},
	{0x1FF00000, 0x1FFFFC00},
	{0x20000000, 0x23FFFE00},
	{0x24000000, (u32)FIRM_BUFFER}
};

bool FirmAddressValid(u32 loc, u32 dest, u32 size)
{
	bool ret=false;
	u32 regs = sizeof(wlist)/sizeof(wlist[0]);
	if (loc&0x1FF || dest&0x3 || size&0x1FF)
		return false;

	for (u32 i = 0; (ret == false) && (i < regs); i++) {
		ret |= (RANGE(dest, wlist[i][0], wlist[i][1]) &&
				RANGE(dest+size, wlist[i][0], wlist[i][1]));
	}
	return ret;
}

bool FirmValid(void *firm, u32 firm_size)
{
	FirmHeader *hdr;
	FirmSection *sect;
	bool mpcoreFound, stdFound;
	u32 firmSizeTotal;
	u8 sectionHash[0x20];
	
	hdr = (FirmHeader*)firm;
	if (firm_size < sizeof(FirmHeader) ||
		memcmp(hdr->magic, FIRM_MAGIC, 4)) {
		return false;
	}

	mpcoreFound = false;
	stdFound = false;
	firmSizeTotal = sizeof(FirmHeader);

	for (int i = 0; i < 4; i++) {
		u32 off, addr, size;
		sect = &(hdr->section[i]);
		off = sect->off;
		addr = sect->addr;
		size = sect->size;

		if (!size)
			continue;

		firmSizeTotal += size;
		if (firmSizeTotal > firm_size || !FirmAddressValid((u32)hdr + off, addr, size))
			return false;

		sha_quick(sectionHash, (void*)((u32)hdr + off), size, SHA256_MODE);
		if (memcmp(sectionHash, sect->shaHash, 0x20)) {
			return false;
		}

		if (!mpcoreFound && RANGE(hdr->mpcoreEntry, addr, addr + size))
			mpcoreFound = true;

		if (!stdFound && RANGE(hdr->stdEntry, addr, addr + size))
			stdFound = true;
	}

	if (!mpcoreFound)
		hdr->mpcoreEntry = 0;

	return stdFound;
}

void LED_3D(void)
{
	I2C_init();
	I2C_writeReg(3, 0x2C, 0x0F);
}

void main(void)
{
	FATFS fs;
	FIL firm;
	FRESULT res;
	size_t flen, br;
	char *boot_path = "sdmc:/"PATH_DEFAULT;
	bool firm_found = 0, _debug = 0;
	
	ClearScreenFull(1,1);
	
	#ifdef DEBUG
	_debug = 1;
	#endif
	
	if (f_mount(&fs, "0:", 0) != FR_OK) PowerOff();
	
	for (u8* addr = (u8*) 0x20000200; addr < (u8*) 0x24000000; addr += 0x400000)
	{	
		
		if (FirmValid((FirmHeader*) (void*) addr, FIRM_MAXSIZE))
		{
			if(_debug) DrawStringF(0, 0, 1, "Valid payload found at 0x%X in FCRAM", addr);
			memmove(FIRM_BUFFER, addr, FIRM_MAXSIZE);
			if (memcmp(addr, "FIRM", 4) == 0) memcpy(addr, "A9NC", 4); // prevent bootloops
			firm_found = 1;
			LED_3D();
			boot_path = "A9SP";
			flen = FIRM_MAXSIZE;
			break;
		}
	}
	
	if((_debug) && (!firm_found))
	{
		DrawStringF(0, 20, 1, "No payload found in FCRAM, attempting to boot");
		DrawStringF(0, 36, 1, "%s instead", boot_path);
	}
	
	if((!firm_found) && (f_open(&firm, PATH_DEFAULT, FA_READ) == FR_OK) && (f_size(&firm) <= FIRM_MAXSIZE))
	{
		flen=f_size(&firm);
		res = f_read(&firm, FIRM_BUFFER, flen, &br);
		f_close(&firm);
		if((flen == br) && (res == FR_OK)) firm_found = 1;
	}
	if(f_size(&firm) > FIRM_MAXSIZE) f_close(&firm);
	
	//*(vu64*) 0x1FFFE010 = 0;
	if((firm_found) && (_debug)) DrawStringF(0, 56, 1, "Booting payload...");
	if ((firm_found) && (FirmValid(FIRM_BUFFER, flen))) BootFirm(FIRM_BUFFER, boot_path);
	else
	{
		if(_debug) DrawStringF(0, 56, 1, "Booting payload...Failed");
	}
	
	while(1);
}
