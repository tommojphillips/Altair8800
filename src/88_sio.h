/* 88_sio.h
 * Serial io board - 88-SIO
 * Github: https:\\github.com\tommojphillips
 */

#ifndef _88_SIO_H
#define _88_SIO_H

#include <stdint.h>

typedef struct {
	uint8_t status;
	uint8_t control;
	uint8_t output_interrupt;
	uint8_t input_interrupt;
	char ch;
} SIO;

void sio_init(SIO* sio);
void sio_update(SIO* sio);

int sio_read_io(SIO* sio, uint8_t port, uint8_t* value);
int sio_write_io(SIO* sio, uint8_t port, uint8_t value);

uint8_t sio_status(SIO* sio);
uint8_t sio_read(SIO* sio);
void sio_write(char ch);
void sio_control(SIO* sio, uint8_t value);

#endif
