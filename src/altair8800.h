/* altair8800.h
 * Github: https:\\github.com\tommojphillips
 */

#ifndef ALTAIR_8800_H
#define ALTAIR_8800_H

#include "i8080.h"

typedef struct {
	uint8_t status;
	char ch;
} SIO;

typedef struct {
	I8080 cpu;
	uint8_t* memory;
	uint32_t ram_size;
	uint8_t front_panel_switches;
	SIO sio;
} ALTAIR8800;

extern ALTAIR8800 altair;

void altair8800_update();
int altair8800_init();
void altair8800_destroy();

#endif
