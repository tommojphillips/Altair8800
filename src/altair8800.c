/* altair8800.c
 * Github: https:\\github.com\tommojphillips
 */

#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "altair8800.h"
#include "i8080.h"

#define REFRESH_RATE 60
#define CPU_CLOCK 2000000 /* 2 Mhz */
#define VBLANK_RATE (CPU_CLOCK / REFRESH_RATE)

ALTAIR8800 altair = { 0 };

uint8_t altair8800_read_byte(uint16_t address) {
	return *(uint8_t*)(altair.memory + (address & 0xFFFF));
}
void altair8800_write_byte(uint16_t address, uint8_t value) {
	if (address < altair.ram_size)
		*(uint8_t*)(altair.memory + (address & 0xFFFF)) = value;
}

#define PORT_SIO_STATUS  0x10
#define PORT_SIO_DATA    0x11

#define PORT_FRONT_PANEL_SWITCHES 0xFF

#define STATUS_2SIO_HAS_INPUT 0x3
#define STATUS_2SIO_NO_INPUT  0x2

#include <conio.h>
int kb_has_input() {
	return _kbhit();
}
void kb_get_input(char* ch) {
	*ch = (char)_getch();
}

uint8_t sio_status() {
	if (kb_has_input()) {
		altair.sio.status = STATUS_2SIO_HAS_INPUT;
	}
	else {
		altair.sio.status = STATUS_2SIO_NO_INPUT;
	}
	return altair.sio.status;
}
uint8_t sio_read() {
	kb_get_input(&altair.sio.ch);
	if (altair.sio.ch >= 'a' && altair.sio.ch <= 'z') altair.sio.ch -= 0x20;
	if (altair.sio.ch == 0x08) altair.sio.ch = '_';
	return altair.sio.ch;
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
	switch (port) {

		case PORT_SIO_STATUS:
			return sio_status();

		case PORT_SIO_DATA:
			return sio_read();

		case PORT_FRONT_PANEL_SWITCHES:
			return altair.front_panel_switches;

		default:
			printf("Reading from undefined port: %02X\n", port);
			return 0;
	}

}
void altair8800_write_io(uint8_t port, uint8_t value) {
	switch (port) {
	
		case PORT_SIO_STATUS:
			break;

		case PORT_SIO_DATA:
			sio_write(value);
			break;

		case PORT_FRONT_PANEL_SWITCHES:
			//altair.front_panel_switches = value;
			break;

		default:
			printf("Writing to undefined port: %02X = %02X\n", port, value);
			break;
	}
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
	altair.ram_size = 0x8000;
	altair.front_panel_switches = 0x00;
	return 0;
}
void altair8800_destroy() {
	if (altair.memory != NULL) {
		free(altair.memory);
		altair.memory = NULL;
	}
}
