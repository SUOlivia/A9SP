# A9SP
ARM9 Shortcut Project

## Notes for compiling for userland:

* Put the payload in the `romfs` folder named `payload.firm`
* Put the splash screen in the `romfs` folder named `splash.bin` (converted using the same method used for luma splashes)
* Edit the makefile for name, description, unique id and product code
* Put banner image (`banner.png`), banner sound (`banner.wav`) and icon (`icon.png`) in the `assets` folder (Placeholder is for GodMode9)
* Bannertool and Makerom needed in the path

## Notes about the chainloader

The chainloader might not have complete compatibility with all payloads, but should work with most payloads

## Installation / Usage

Rename `/boot.firm` to `/boot0.firm` and copy the chainloader to `boot.firm`
For those who have GodMode9's bootloader mode installed as Firm0/Firm1, you don't need to do the above

Credits:

[d0k3](https://github.com/d0k3) and [Wolfvak](https://github.com/Wolfvak) for OldLoader and useful help

[MarcuzD](https://github.com/MarcuzD) for the Makefile
