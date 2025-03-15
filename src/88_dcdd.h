/* 88_dcdd.h
 * Altair Floppy Disk - 88-DCDD
 * Github: https:\\github.com\tommojphillips
 */

#ifndef _88_DCDD_H
#define _88_DCDD_H

#include <stdint.h>
#include <stdio.h>

#define DCDD_MAX_DISKS 16

typedef struct {
	uint8_t status;
	uint8_t command;
	uint8_t sector_pos;
	uint8_t track_pos;
	int8_t selector;
	uint32_t read_index;
	FILE* disk_file[DCDD_MAX_DISKS];
} DCDD;

void dcdd_init(DCDD* dcdd);

int dcdd_read_io(DCDD* dcdd, uint8_t port, uint8_t* value);
int dcdd_write_io(DCDD* dcdd, uint8_t port, uint8_t value);

uint8_t dcdd_status(DCDD* dcdd);
uint8_t dcdd_read(DCDD* dcdd);
void dcdd_write(DCDD* dcdd, uint8_t value);
uint8_t dcdd_sector_pos(DCDD* dcdd);
void dcdd_selector(DCDD* dcdd, uint8_t value);
void dcdd_command(DCDD* dcdd, uint8_t value);

#endif
