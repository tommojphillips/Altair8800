/* altair8800.h
 * Github: https:\\github.com\tommojphillips
 */

#ifndef ALTAIR_8800_H
#define ALTAIR_8800_H

#include "i8080.h"
#include "88_sio.h"
#include "88_dcdd.h"

typedef struct {
	I8080 cpu;
	uint8_t* memory;
	uint32_t ram_size;
	uint8_t front_panel_switches;
	SIO sio;
	DCDD dcdd;
	int running;
} ALTAIR8800;

extern ALTAIR8800 altair;

void altair8800_update();
int altair8800_init();
void altair8800_destroy();

#endif
