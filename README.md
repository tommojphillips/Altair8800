# Altair 8800 Emulator

A MITS Altair 8800 emulator that runs in the terminal. Capable of running MITS BASIC 8k

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
 Altair8800.exe -o<offset> <rom>
 ```
 ---

 |  Options       |  Desc      | Default      |
 | -------        | ---------- | ------------ |
 | `-o<offset>`   | Offset     | 0x0000       |
 | `-r<size>`     | Ram size   | 0x8000 (32K) |

  - Offset should be in hex
  - Programs are deposited into memory sequentially starting from `-o<offset>`

 ---

### Running Wozmon for i8080
 - Download Wozmon rom from https://github.com/tommojphillips/Wozmon-i8080/
 - Wozmon for i8080 should be loaded at hex address: `D000`

 ```
 Altair8800.exe -oD000 wozmon.bin
 ```

![Screenshot 2025-03-09 132652](https://github.com/user-attachments/assets/8bc71740-ed72-4a07-8ded-9e4e055184bd)

### Running Altair Basic 8K
 - Download 8K Basic roms from https://altairclone.com/downloads/roms/8K%20BASIC/
 - Altair Basic should be loaded at hex address: `E000`

 ```
 Altair8800.exe -oE000 8kBas_e0.bin 8kBas_e8.bin 8kBas_f0.bin 8kBas_f8.bin
 ```
![Screenshot 2025-03-09 132740](https://github.com/user-attachments/assets/2fa1785d-958a-4534-a5b9-a9286847b626)

