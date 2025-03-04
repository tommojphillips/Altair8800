/* altair8800.c
 * Github: https:\\github.com\tommojphillips
 */

#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <conio.h>

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

#define STATUS_SIO_HAS_INPUT 0x3
#define STATUS_SIO_NO_INPUT  0x2

#define SIO_OUTPUT_DEVICE_READY  0x80 // ACTIVE LOW  - A ready pulse has been sent from the device ( Also causes a hardware interrupt to occur if interrupt enabled. )
                               //0x40 // NOT USED
#define SIO_DATA_AVAILABLE       0x20 // ACTIVE HIGH - Data Available ( a word of data is in the buffer on the 1 / 0 board )
#define SIO_DATA_OVERFLOW        0x10 // ACTIVE HIGH - Data Overflow ( a new word of data has been recieved before the previous word was inputed to the accumulator )
                               //0x08 // ACTIVE HIGH - Framming Error ( data word has no valid stop bit )
                               //0x04 // ACTIVE HIGH - Parity Error ( recieved parity does not aggree with selected parity )
#define SIO_DATA_EMPTY           0x02 // ACTIVE HIGH - X-mitter Buffer Empty ( the previous data word has been X - mitted and a new data word may be outputted )
#define SIO_INPUT_DEVICE_READY   0x01 // ACTIVE LOW  - A ready pulse has been sent from the device

int kb_has_input() {
	return _kbhit();
}
void kb_get_input(char* ch) {
	*ch = (char)_getch();
}

void sio_update() {
	if (kb_has_input()) {
		if (altair.sio.ch != 0) {
			altair.sio.status |= SIO_DATA_OVERFLOW;
		}
		else {
			altair.sio.status &= ~SIO_DATA_OVERFLOW;
		}
		kb_get_input(&altair.sio.ch);
		altair.sio.status |= SIO_INPUT_DEVICE_READY;
		altair.sio.status &= ~SIO_DATA_EMPTY;
		altair.sio.status |= SIO_DATA_AVAILABLE;
	}
	else {
		altair.sio.status &= ~SIO_INPUT_DEVICE_READY;
	}
}
uint8_t sio_status() {
	return altair.sio.status;
}
uint8_t sio_read() {
	char ch = altair.sio.ch;
	altair.sio.ch = 0;
	if (ch >= 'a' && ch <= 'z') { 
		ch -= 0x20;
	}
	if (ch == 0x08) {
		ch = '_';
	}
	altair.sio.status &= ~SIO_INPUT_DEVICE_READY;
	altair.sio.status |= SIO_DATA_EMPTY;
	altair.sio.status &= ~SIO_DATA_AVAILABLE;
	return ch;
}
void sio_write(char ch) {
	if (ch == '_') {
		printf("\b \b");
	}
	else {
		printf("%c", ch);
	}
}
void sio_control(uint8_t value) {
	/* The control channel has two purposes: it is used to enable/disable the hardware
	interrupt capability for the Input or Output device, and to test the status of the
	Input/Output device. 
	
	After an "IN" instruction is executed with the control channel address, "SINP" goes
	high and IC J pin 4 is high thus causing IC G pin 3 to go low. This enables SW
	(Status Word Enable) at IC M pin 16 and causes IC E pin 12 and IC D pin 8 to go low,
	thus enabling the Data In lines. (Note that IC D pin 12 is always high except during
	the into initial the CPU power on clear, POC.) This inputs the data to the Data In lines and
	into the CPU accumulator. 
	
	When an "OUT" instruction is executed with the control channel address, data bits
	0 & 1 are gated through IC's E & A to the Input/Output interrupt flip-flops, IC B. 

	D0     D1     Output interrupt  Input interrupt
	 low    low    0                 0
	 low    high   1                 0
	 high   low    0                 1
	 high   high   1                 1
	*/

	altair.sio.control = value & 0x03;
	altair.sio.input_interrupt = value & 0x01;
	altair.sio.output_interrupt = (value & 0x02) >> 1;
}
void sio_device_select(uint8_t value) {
	/* When the CPU executes an "OUT" or an "IN" instruction, it places the device address 
	( provided with the instruction ) on both the 8 lower order address bus lines and the 
	8 higher order address bus lines. 
	
	The 8 lower order address bus lines are fed to the select logic on the board, IC's
	H & J. If the address on the bus is equal to the address selected on the board,
	IC I pin 8 will go low,. thus enabling IC J pins 3 & 6.
	
	Depending on the state of AO (the least significant address bit), either the control 
	pin channel 4 or the data channel will be enabled. If AO is at a logic low level, IC J
	will go high, thus enabling the control channel. If AO is at a logic high
	level, ICJ pin I will go high, thus enabling the data channel.
	Of number the two device addresses on the board, the control channel is always an even 
	and the data channel is always an odd number. */
}
void sio_control(uint8_t value) {
	/* The control channel has two purposes: it is used to enable/disable the hardware
	interrupt capability for the Input or Output device, and to test the status of the
	Input/Output device. 
	
	After an "IN" instruction is executed with the control channel address, "SINP" goes
	high and IC J pin 4 is high thus causing IC G pin 3 to go low. This enables SW
	(Status Word Enable) at IC M pin 16 and causes IC E pin 12 and IC D pin 8 to go low,
	thus enabling the Data In lines. (Note that IC D pin 12 is always high except during
	the into initial the CPU power on clear, POC.) This inputs the data to the Data In lines and
	into the CPU accumulator. 
	
	When an "OUT" instruction is executed with the control channel address, data bits
	0 & 1 are gated through IC's E & A to the Input/Output interrupt flip-flops, IC B. 

	D0     D1     Output interrupt  Input interrupt
	 low    low    0                 0
	 low    high   1                 0
	 high   low    0                 1
	 high   high   1                 1
	*/

	altair.sio.control = value & 0x03;
	altair.sio.input_interrupt = value & 0x01;
	altair.sio.output_interrupt = (value & 0x02) >> 1;
}
void sio_device_select(uint8_t value) {
	/* When the CPU executes an "OUT" or an "IN" instruction, it places the device address 
	( provided with the instruction ) on both the 8 lower order address bus lines and the 
	8 higher order address bus lines. 
	
	The 8 lower order address bus lines are fed to the select logic on the board, IC's
	H & J. If the address on the bus is equal to the address selected on the board,
	IC I pin 8 will go low,. thus enabling IC J pins 3 & 6.
	
	Depending on the state of AO (the least significant address bit), either the control 
	pin channel 4 or the data channel will be enabled. If AO is at a logic low level, IC J
	will go high, thus enabling the control channel. If AO is at a logic high
	level, ICJ pin I will go high, thus enabling the data channel.
	Of number the two device addresses on the board, the control channel is always an even 
	and the data channel is always an odd number. */
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
			sio_control(value);
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
	sio_update();
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
	
	altair.sio.status = 0;
	altair.sio.status |= SIO_DATA_EMPTY;
	
	return 0;
}
void altair8800_destroy() {
	if (altair.memory != NULL) {
		free(altair.memory);
		altair.memory = NULL;
	}
}
