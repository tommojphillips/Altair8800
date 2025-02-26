# Altair 8800 Emulator

A MITS Altair 8800 emulator that runs in the terminal. Capable of running MITS BASIC 8k

## Building

The project is built in Visual Studio 2022
 
 1. Clone the repo

```
git clone --recurse-submodules https://github.com/tommojphillips/Altair8800.git
```

 2. Open `vs\Altair8800.sln`  in visual studio, build and run

## ROMS
 |                 |                                                         |
 | --------------- | -------------------------------------------------       |
 | Altair Basic 8K | https://altairclone.com/downloads/roms/8K%20BASIC/      |
 | Wozmon          | https://github.com/tommojphillips/Wozmon-i8080/releases |

### Running Altair Basic 8K

![Screenshot 2025-02-26 205416](https://github.com/user-attachments/assets/90ce99c7-e201-40f9-934b-2b208d9680b2)

If Wozmon i8080 is used, The emualator will startup in wozmon. Examine Hex Address `E000` and Run 
```
\
E000 R
```

