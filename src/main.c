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
				offset = strtol(arg + 2, NULL, 16) & 0xFFFF;
				altair.cpu.pc = offset;
				break;
			}

			if (strncmp("-r", arg, 2) == 0) {
				altair.ram_size = strtol(arg + 2, NULL, 16) & 0xFFFF;
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
						disk = strtol(arg, NULL, 10) & 0xF;
					}
					arg += 2;
				}

				altair.dcdd.disk_file[disk] = NULL;
				fopen_s(&altair.dcdd.disk_file[disk], arg, "r+b");
				if (altair.dcdd.disk_file[disk] == NULL) {
					printf("Failed to open disk file: %s\n", arg);
				}
				else {
					printf("%s -> %c:\n", arg, 'A'+disk);
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
