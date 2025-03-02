# Altair 8800 Emulator

A MITS Altair 8800 emulator that runs in the terminal. Capable of running MITS BASIC 8k

## Building

The project is built in Visual Studio 2022
 
 1. Clone the repo

```
git clone --recurse-submodules https://github.com/tommojphillips/Altair8800.git
```

 2. Open `vc\Altair8800.sln`  in visual studio, build and run


 ## Usage
 
  Syntax:
 ```
 Altair8800.exe -o<offset> -f<filename>
 ```
 ---

  - Offset should be in hex

 Options:
 | Opt     |  Desc      | Default      |
 | ------- | ---------- | ------------ |
 | `-o`    | Offset     | 0x0000       |
 | `-f`    | Filename   |              |
 | `-r`    | Ram amount | 0x8000 (32K) |

 ---

### Running Wozmon for i8080
 - Download Wozmon rom from https://github.com/tommojphillips/Wozmon-i8080/
 - Wozmon for i8080 should be loaded at hex address: `D000`
 - Use command:
 ```
 Altair8800.exe -oD000 -fwozmon.bin
 ```

### Running Altair Basic 8K
 - Download 8K Basic roms from https://altairclone.com/downloads/roms/8K%20BASIC/
 - Altair Basic should be loaded at hex address: `E000`
 - Use command:
 ```
 Altair8800.exe -oE000 -f8kBas_e0.bin -f8kBas_e8.bin -f8kBas_f0.bin -f8kBas_f8.bin
 ```
