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
		uint32_t len = (uint32_t)strlen(argv[i]);
		int next_arg = 0;
		for (int j = 0; j < (int)len;) {
			switch (argv[i][j]) {
				case 'f':
					read_file_into_buffer(argv[i] + j + 1, altair.memory, 0x10000, offset, &len, 0);
					offset += len;
					next_arg = 1;
					break;
				case 'o':
					offset = strtol(argv[i] + j + 1, NULL, 16) & 0xFFFF;
					altair.cpu.pc = offset;
					next_arg = 1;
					break;
				case 'r':
					altair.ram_size = strtol(argv[i] + j + 1, NULL, 16) & 0xFFFF;
					next_arg = 1;
					break;
				case 'p':
					clear_console_mode(ENABLE_PROCESSED_INPUT);
					next_arg = 1;
					break;

				case '-':
				case '/':
				default:
					++j;
					break;
			}
			if (next_arg) break;
		}
	}
}


int main(int argc, char** argv) {
	altair8800_init();
	args(argc, argv);
	while (1) {
		altair8800_update();
	}
	altair8800_destroy();
	return 0;
}
