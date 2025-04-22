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
#include "88_sio.h"
#include "88_dcdd.h"

#define REFRESH_RATE 60
#define CPU_CLOCK 2000000 /* 2 Mhz */
#define VBLANK_RATE (CPU_CLOCK / REFRESH_RATE)

#define PORT_FRONT_PANEL_SWITCHES 0xFF

#ifdef _DEBUG
#define dbg_err(x, ...) printf(x, __VA_ARGS__)
#else
#define dbg_err(x, ...)
#endif

ALTAIR8800 altair = { 0 };

uint8_t altair8800_read_byte(uint16_t address) {
	return *(uint8_t*)(altair.memory + (address & 0xFFFF));
}
void altair8800_write_byte(uint16_t address, uint8_t value) {
	if (address < altair.ram_size)
		*(uint8_t*)(altair.memory + (address & 0xFFFF)) = value;
}

uint8_t altair8800_read_io(uint8_t port) {
	uint8_t value;
	if (port == PORT_FRONT_PANEL_SWITCHES) {
		return altair.front_panel_switches;
	}
	else if (dcdd_read_io(&altair.dcdd, port, &value)) {
		return value;
	}
	else if (sio_read_io(&altair.sio, port, &value)) {
		return value;
	}
	else {
		dbg_err("Reading from undefined port: %02X\n", port);
		return 0xFF;
	}
}
void altair8800_write_io(uint8_t port, uint8_t value) {
	if (port == PORT_FRONT_PANEL_SWITCHES) {

	}
	else if (dcdd_write_io(&altair.dcdd, port, value)) {
		return;
	}
	else if (sio_write_io(&altair.sio, port, value)) {
		return;
	}
	else {
		dbg_err("Writing to undefined port: %02X = %02X\n", port, value);
	}
}

void push_word(I8080* cpu, uint16_t value);

static void altair8800_interrupt(uint8_t rst_num) {
	if (altair.cpu.flags.interrupt) {
		altair.cpu.flags.interrupt = 0;
		push_word(&altair.cpu, altair.cpu.pc);
		altair.cpu.pc = (rst_num & 0b111) << 3;
	}
}
void altair8800_update() {
	altair.cpu.cycles = 0;
	while (altair.cpu.cycles < VBLANK_RATE) {
		i8080_execute(&altair.cpu);
	}
	sio_update(&altair.sio);
	if (altair.sio.ch == 0x1B) {
		altair.running = 0;
	}
}

int altair8800_init() {
	altair.memory = (uint8_t*)malloc(0x10000);
	if (altair.memory == NULL) {
		dbg_err("Failed to allocate Memory\n");
		return 1;
	}
	memset(altair.memory, 0, 0x10000);

	i8080_init(&altair.cpu);
	altair.cpu.read_byte = altair8800_read_byte;
	altair.cpu.write_byte = altair8800_write_byte;
	altair.cpu.read_io = altair8800_read_io;
	altair.cpu.write_io = altair8800_write_io;
	
	altair.ram_size = 0x10000;
	altair.front_panel_switches = 0x00;
	altair.running = 1;
	
	sio_reset(&altair.sio);
	dcdd_init(&altair.dcdd);
	dcdd_reset(&altair.dcdd);
	return 0;
}
void altair8800_destroy() {
	if (altair.memory != NULL) {
		free(altair.memory);
		altair.memory = NULL;
	}

	dcdd_free(&altair.dcdd);
}
