/* 88_sio.c
 * Serial io board - 88-SIO
 * Github: https:\\github.com\tommojphillips
 */

#include <stdint.h>
#include <stdio.h>
#include <conio.h>

#include "88_sio.h"

#define SIO_OUTPUT_DEVICE_READY  0x80 // ACTIVE LOW  - A ready pulse has been sent
#define SIO_DATA_AVAILABLE       0x20 // ACTIVE HIGH - Data Available
#define SIO_DATA_OVERFLOW        0x10 // ACTIVE HIGH - Data Overflow
#define SIO_DATA_EMPTY           0x02 // ACTIVE HIGH - X-mitter Buffer Empty
#define SIO_INPUT_DEVICE_READY   0x01 // ACTIVE LOW  - A ready pulse has been sent

#define PORT_SIO_STATUS    0x10 // 88-sio status port (in)
#define PORT_SIO_DATA      0x11 // 88-sio data port (in/out)

void sio_init(SIO* sio) {
	sio->status = 0;
	sio->status |= SIO_DATA_EMPTY;
}
void sio_update(SIO* sio) {
	if (_kbhit()) {
		if (sio->ch != 0) {
			sio->status |= SIO_DATA_OVERFLOW;
		}
		else {
			sio->status &= ~SIO_DATA_OVERFLOW;
		}
		sio->ch = (char)_getch();		
		sio->status |= SIO_INPUT_DEVICE_READY;
		sio->status &= ~SIO_DATA_EMPTY;
		sio->status |= SIO_DATA_AVAILABLE;
	}
	else {
		sio->status &= ~SIO_INPUT_DEVICE_READY;
	}
}

int sio_read_io(SIO* sio, uint8_t port, uint8_t* value) {
	switch (port) {

		case PORT_SIO_STATUS:
			*value = sio_status(sio);
			break;

		case PORT_SIO_DATA:
			*value = sio_read(sio);
			break;

		default:
			return 0;
	}
	return 1;
}
int sio_write_io(SIO* sio, uint8_t port, uint8_t value) {
	switch (port) {

		case PORT_SIO_STATUS:
			sio_control(sio, value);
			break;

		case PORT_SIO_DATA:
			sio_write(value);
			break;

		default:
			return 0;
	}
	return 1;
}

uint8_t sio_status(SIO* sio) {
	return sio->status;
}
uint8_t sio_read(SIO* sio) {
	char ch = sio->ch;
	sio->ch = 0;
	if (ch >= 'a' && ch <= 'z') {
		ch -= 0x20;
	}
	if (ch == 0x08) {
		ch = '_';
	}
	sio->status &= ~SIO_INPUT_DEVICE_READY;
	sio->status |= SIO_DATA_EMPTY;
	sio->status &= ~SIO_DATA_AVAILABLE;
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
void sio_control(SIO* sio, uint8_t value) {
	sio->control = value & 0x03;
	sio->input_interrupt = value & 0x01;
	sio->output_interrupt = (value & 0x02) >> 1;
}
