/* altair8800.c
 * Github: https:\\github.com\tommojphillips
 */

#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "file.h"
#include "altair8800.h"
#include "i8080.h"

#define REFRESH_RATE 60
#define CPU_CLOCK 2000000 /* 2 Mhz */
#define VBLANK_RATE (CPU_CLOCK / REFRESH_RATE)

int bas8k_init();
int wozmon_init();

typedef struct {
	I8080 cpu;
	uint8_t* memory;

	uint8_t status;

} ALTAIR8800;

ALTAIR8800 altair = { 0 };

int bas8k_init() {
	if (read_file_into_buffer("8kBas_e0.bin", altair.memory, 0x10000, 0xE000, 0x800) != 0) return 1;
	if (read_file_into_buffer("8kBas_e8.bin", altair.memory, 0x10000, 0xE800, 0x800) != 0) return 1;
	if (read_file_into_buffer("8kBas_f0.bin", altair.memory, 0x10000, 0xF000, 0x800) != 0) return 1;
	if (read_file_into_buffer("8kBas_f8.bin", altair.memory, 0x10000, 0xF800, 0x800) != 0) return 1;
	return 0;
}
int wozmon_init() {
	if (read_file_into_buffer("wozmon.bin", altair.memory, 0x10000, 0xD000, 0) != 0) return 1;
	return 0;
}

uint8_t altair8800_read_byte(uint16_t address) {
	return *(uint8_t*)(altair.memory + (address & 0xFFFF));
}
void altair8800_write_byte(uint16_t address, uint8_t value) {
	if (address < 0x8000)
		*(uint8_t*)(altair.memory + (address & 0xFFFF)) = value;
}

#define PORT_SIO_STATUS  0x10
#define PORT_SIO_READ    0x11
#define PORT_SIO_CONTROL 0x10
#define PORT_SIO_WRITE   0x11

#define PORT_FRONT_PANEL_SWITCHES 0xFF

#include <conio.h>
int kb_has_input() {
	return _kbhit();
}
void kb_get_input(char* ch) {
	*ch = (char)_getch();
}

#define STATUS_2SIO_HAS_INPUT 0x3
#define STATUS_2SIO_NO_INPUT  0x2

uint8_t sio_status() {
	if (kb_has_input()) {
		return STATUS_2SIO_HAS_INPUT;
	}
	else {
		return STATUS_2SIO_NO_INPUT;
	}
}
void sio_read(char* ch) {
	kb_get_input(ch);
	if (*ch >= 'a' && *ch <= 'z') *ch -= 0x20;
	if (*ch == 0x08) *ch = '_';
}
void sio_write(char ch) {
	if (ch == '_') {
		printf("\b \b");
	}
	else {
		printf("%c", ch);
	}
}

uint8_t altair8800_read_io(uint8_t port) {
	static char ch = 0;
	switch (port) {

		case PORT_SIO_STATUS:
			return sio_status();

		case PORT_SIO_READ:
			sio_read(&ch);
			return ch;

		case PORT_FRONT_PANEL_SWITCHES:
			return 0x0;

		default:
			//printf("Reading from undefined port: %02X\n", port);
			return 0;
	}

}
void altair8800_write_io(uint8_t port, uint8_t value) {
	switch (port) {
	
		case PORT_SIO_WRITE:
			sio_write(value);
			break;

		case PORT_FRONT_PANEL_SWITCHES:
			break;

		default:
			//printf("Writing to undefined port: %02X = %02X\n", port, value);
			break;
	}
}

void altair8800_reset() {
	i8080_reset(&altair.cpu);
	altair.cpu.pc = 0xD000;
	memset(altair.memory, 0, 0xE000);
}
void altair8800_update() {
	while (altair.cpu.cycles < VBLANK_RATE) {
		i8080_execute(&altair.cpu);
	}
	altair.cpu.cycles = 0;
}

int altair8800_init() {
	altair.memory = (uint8_t*)malloc(0x10000);
	if (altair.memory == NULL) {
		printf("Failed to allocate Memory\n");
		return 1;
	}
	memset(altair.memory, 0, 0x10000);

	i8080_init(&altair.cpu);
	altair.cpu.read_byte = altair8800_read_byte;
	altair.cpu.write_byte = altair8800_write_byte;
	altair.cpu.read_io = altair8800_read_io;
	altair.cpu.write_io = altair8800_write_io;

	altair8800_reset();

	wozmon_init();
	bas8k_init();
	return 0;
}
void altair8800_destroy() {
	if (altair.memory != NULL) {
		free(altair.memory);
		altair.memory = NULL;
	}
}
