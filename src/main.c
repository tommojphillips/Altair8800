/* main.c
 * Github: https:\\github.com\tommojphillips
 */

#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "i8080.h"
#include "altair8800.h"
#include "file.h"

void clear_console_mode(uint32_t mode_mask) {
	DWORD mode = 0;
	HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(input_handle, &mode);
	mode &= ~mode_mask;
	SetConsoleMode(input_handle, mode);
}

void args(int argc, char** argv) {
	const char* filename = NULL;
	uint32_t offset = 0;
	for (int i = 1; i < argc; ++i) {
		size_t len = strlen(argv[i]);
		for (size_t j = 0; j < len;) {
			const char* arg = argv[i] + j;

			if (strncmp("-o", arg, 2) == 0) {
				offset = strtol(arg + 2, NULL, 16);
				altair.cpu.pc = offset & 0xFFFF;
				break;
			}

			if (strncmp("-r", arg, 2) == 0) {
				altair.ram_size = strtol(arg + 2, NULL, 16) & 0xFFFF;
				printf("%04X\t-> RAMTOP\n", altair.ram_size);
				break;
			}

			if (strncmp("-p", arg, 2) == 0) {
				clear_console_mode(ENABLE_PROCESSED_INPUT);
				break;
			}

			if (strncmp("-d", arg, 2) == 0) {
				arg += 2; 
				uint8_t disk = 0; 
				if (arg[1] == ':') {
					if ((arg[0] >= 'A' && arg[0] <= 'Z')) {
						disk = arg[0] - 'A';
					}
					else if (arg[0] >= 'a' && arg[0] <= 'z') {
						disk = arg[0] - 'a';
					}
					else {
						disk = strtol(arg, NULL, 10) & 0xFF;
					}
					arg += 2;
					disk &= 0xF; // map disk A-P (0-15)
				}

				if (altair.dcdd.disks[disk].file != NULL) {
					fclose(altair.dcdd.disks[disk].file);
				}
				altair.dcdd.disks[disk].file = NULL;
				fopen_s(&altair.dcdd.disks[disk].file, arg, "r+b");
				if (altair.dcdd.disks[disk].file == NULL) {
					printf("Failed to open disk file: %s\n", arg);
				}
				else {
					printf("%c:\t-> %s\n", 'A'+disk, arg);
				}
				break;
			}

			uint32_t file_size = 0;
			read_file_into_buffer(arg, altair.memory, 0x10000, offset, &file_size, 0);
			offset += file_size;
			break;
		}
	}
}

int main(int argc, char** argv) {
	altair8800_init();
	args(argc, argv);
	while (altair.running) {
		altair8800_update();
	}
	altair8800_destroy();
	return 0;
}
