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
	uint8_t status; // disk status
	uint8_t sector; // sector position
	uint8_t track;  // track position
	uint32_t index; // track index 
	FILE* file;
} DISK;

typedef struct {
	int8_t selector; // disk selector
	DISK* disks;     // disks (16)
} DCDD;

int dcdd_init(DCDD* dcdd);
void dcdd_free(DCDD* dcdd);
void dcdd_reset(DCDD* dcdd);

int dcdd_read_io(DCDD* dcdd, uint8_t port, uint8_t* value);
int dcdd_write_io(DCDD* dcdd, uint8_t port, uint8_t value);

#endif
