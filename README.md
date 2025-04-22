# Altair 8800 Emulator

A MITS Altair 8800 emulator that runs in the terminal. Capable of booting CPM 3 from floppy disk. 8" Floppies (.dsk 329K) are supported.

## Building

The project is built in Visual Studio 2022
 
| Dependencies   |                                                                |
| -------------- | -------------------------------------------------------------- |
| I8080          | https://github.com/tommojphillips/i8080                        |

 1. Clone the repo

```
git clone --recurse-submodules https://github.com/tommojphillips/Altair8800.git
```

 2. Open `vc\Altair8800.sln`  in visual studio, build and run

 ## Usage
 
Syntax:
 ```
 altair.exe -o<offset> <rom> -d<letter>:<disk_path>
 ```
 ---

 |  Options       |  Desc                   | Default      |
 | -------        | ----------------------- | ------------ |
 | `-o<offset>`   | Offset                  | 0x0000       |
 | `-r<size>`     | Ram size                | 0x8000 (32K) |
 | `-d<letter>`   | Floppy Disk Img (A - P) |              |

  - Offset should be in hex
  - Programs are deposited into memory sequentially starting from `-o<offset>`

 ---

### booting from floppy using Disk Bootloader (DBL) ROM
 - Download Disk Bootloader, `88dskrom.bin` from https://github.com/tommojphillips/Altair8800/tree/master/roms 
 - Load Disk Bootloader (DBL) at `FF00`
 - Load a bootable 8" floppy disk file (329K) into A drive `-dA:`
 ```
 altair.exe -oFF00 88dskrom.bin -dA:<bootable_disk_file>
 ```

#### ROMS
 | Load Offset | Filename | Description                | Link                                                          |
 | ----------- | -------- | -------------------------- |-------------------------------------------------------------- |
 | `FF00`      | 88dskrom.bin | Disk Bootloader (DBL) | https://github.com/tommojphillips/Altair8800/tree/master/roms  |
 | `F800`      | altmon.bin   | Altair Monitor        | https://github.com/tommojphillips/Altair8800/tree/master/roms  |
 | `FD00`      | turnmon.bin  | Turnkey Monitor       | https://github.com/tommojphillips/Altair8800/tree/master/roms  |
 | `D000`      | wozmon.bin   | Wozmon for i8080      | https://github.com/tommojphillips/Wozmon-i8080/                |
 | `E000` `E800` `F000` `F800`      | 8kBas_e0.bin 8kBas_e8.bin 8kBas_f0.bin 8kBas_f8.bin | Altair Basic 8K       | https://altairclone.com/downloads/roms/8K%20BASIC/             |

#### 8" Floppies 329K .dsk
 | Filename       | Description                | Link                                                          |
 | -------------- | -------------------------- |-------------------------------------------------------------- |
 | cpm3_disk1.dsk | CPM v3 disk 1 | https://github.com/tommojphillips/Altair8800/tree/master/roms  |
 | cpm3_disk2.dsk | CPM v3 disk 2 | https://github.com/tommojphillips/Altair8800/tree/master/roms  |

### Disk Bootloader (DBL) ROM
 ```
 altair.exe -oFF00 88dskrom.bin
 ```

### Altair Basic 8K ROM
 ```
 altair.exe -oE000 8kBas_e0.bin 8kBas_e8.bin 8kBas_f0.bin 8kBas_f8.bin
 ```

### WOZMON, ALTMON, CPM 3 8" Floppy (329K .dsk)
Boots into wozmon. 
- type `FF00R` to boot cpm3 from floppy
- type `F800R` to jump to Altair Monitor
```
altair.exe -oFF00 88dskrom.bin -oF800 altmon.bin -oD000 wozmon.bin -dA:cpm3_disk1.dsk -dB:cpm3_disk2.dsk
```
