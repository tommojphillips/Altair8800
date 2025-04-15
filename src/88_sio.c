/* 88_sio.c
 * Serial io board - 88-SIO
 * Github: https:\\github.com\tommojphillips
 */

 /* The board provides serial communication between the ALTAIR and any serial Input/Output devices. 
  * The board has two device code addresses which are hardware selectable by jumpers for any even address from 0x00 to 0xFE.
  * The board provides both hardware and software interrupt capability.
  
 - CONTROL CHANNEL:
	The control channel has two purposes: it is used to enable/disable the hardware interrupt capability for the input or output device,
	and to test the status of the input/output device. 

 - STATUS BYTE (in)

		  7   6   5   4   3   2   1   0
		+---+---+---+---+---+---+---+---+
		| W | X | A | O | F | P | E | R |
		+---+---+---+---+---+---+---+---+

		W - ACTIVE_LOW  - output device is ready (a ready pulse has been sent from the device)
		X - not used
		A - ACTIVE HIGH - data Available  (data is in the buffer on the io board)
		O - ACTIVE HIGH - data overflow  (new data has been recieved before the previous data was inputed to A)
		F - ACTIVE_HIGH - framming error (no valid stop bit)
		P - ACTIVE_HIGH - parity error (recieved parity does not equal selected parity)
		E - ACTIVE HIGH - data buffer is empty (the data has been X-mitted and new data may be outputted)
		R - ACTIVE_LOW  - input device is ready (a ready pulse has been sent from the device)
 
 - STATUS BYTE (out)
	When an "OUT" instruction is executed with the status address, data bits
	0 & 1 are gated to the input/output interrupt flip-flops,

	|  DO  |  D1  | OUTPUT INT |  INPUT INT |
	| LOW  | LOW  |  disabled  |  disabled  |
	| LOW  | HIGH |  enabled   |  disabled  |
	| HIGH | LOW  |  disabled  |  enabled   |
	| HIGH | HIGH |  enabled   |  enabled   |
*/

#include <stdint.h>
#include <stdio.h>
#include <conio.h>

#include "88_sio.h"

#define SIO_OUTPUT_DEVICE_READY  0x80 // ACTIVE LOW  - output device is ready
#define SIO_DATA_AVAILABLE       0x20 // ACTIVE HIGH - data Available
#define SIO_DATA_OVERFLOW        0x10 // ACTIVE HIGH - data Overflow
#define SIO_DATA_EMPTY           0x02 // ACTIVE HIGH - data buffer is empty
#define SIO_INPUT_DEVICE_READY   0x01 // ACTIVE LOW  - input device is ready

#define PORT_SIO_STATUS          0x10 // status port (in)
#define PORT_SIO_CONTROL         0x10 // status port (out)
#define PORT_SIO_DATA            0x11 // data port (in/out)
#define PORT_SIO_DATA1           0x01 // data port (in/out)

void sio_reset(SIO* sio) {
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

		case PORT_SIO_CONTROL:
			sio_control(sio, value);
			break;

		case PORT_SIO_DATA:
			sio_write(value);
			break;

		case PORT_SIO_DATA1:
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
	sio->status &= ~SIO_INPUT_DEVICE_READY;
	sio->status |= SIO_DATA_EMPTY;
	sio->status &= ~SIO_DATA_AVAILABLE;
	return ch;
}
void sio_write(char ch) {
	if (ch == 0x08) {
		printf("\b");
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
