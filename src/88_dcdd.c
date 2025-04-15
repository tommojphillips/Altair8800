/* 88_dcdd.c
 * Altair Floppy Disk - 88-DCDD
 * Github: https:\\github.com\tommojphillips
 */

#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "88_dcdd.h"

 /* SECTOR BYTE
 
		  7   6   5   4   3   2   1   0
		+---+---+---+---+---+---+---+---+
		| X | X | S | S | S | S | S | T |
		+---+---+---+---+---+---+---+---+

		X - ACTIVE_HIGH - not used
		S               - sector number currently under the head, 0-31. (5 bits)
		T - ACTIVE_LOW  - sector true; the sector is positioned to read or write.

		If head is unloaded, hardware returns 0xFF.
 */

 /* STATUS BYTE
 
		  7   6   5   4   3   2   1   0
		+---+---+---+---+---+---+---+---+
		| R | Z | I | X | S | H | M | W |
		+---+---+---+---+---+---+---+---+

		R - ACTIVE_LOW  - read device is ready
		Z - ACTIVE_LOW  - head is on track 0
		I - ACTIVE_LOW  - interrupts are enabled
		X - ACTIVE_HIGH - not used
		S - ACTIVE_LOW  - disk is selected and active
		H - ACTIVE_LOW  - head is loaded for r/w
		M - ACTIVE_LOW  - head can be moved
		W - ACTIVE_LOW  - write device is ready
 */

#define DCDD_TRACKS_PER_DISK       77   // Number of tracks per disk
#define DCDD_SECTORS_PER_TRACK     32   // Number of sectors per track
#define DCDD_BYTES_PER_SECTOR      137  // Number of bytes per sector

#define DCDD_STATUS_WRITE_READY    0x01 // ACTIVE_LOW - write device is ready
#define DCDD_STATUS_MOVE_HEAD      0x02 // ACTIVE_LOW - head can be moved
#define DCDD_STATUS_HEAD_LOADED    0x04 // ACTIVE_LOW - head is loaded for r/w
#define DCDD_STATUS_DRV_SELECT     0x08 // ACTIVE_LOW - disk is selected and active
#define DCDD_STATUS_INT_ENABLED    0x20 // ACTIVE_LOW - interrupts are enabled
#define DCDD_STATUS_TRACK_ZERO     0x40 // ACTIVE_LOW - head is on track 0
#define DCDD_STATUS_READ_READY     0x80 // ACTIVE_LOW - read device is ready

#define DCDD_CMD_STEP_IN           0x01 // step in head command
#define DCDD_CMD_STEP_OUT          0x02 // step out head command
#define DCDD_CMD_LOAD_HEAD         0x04 // load head command
#define DCDD_CMD_UNLOAD_HEAD       0x08 // unload head command
#define DCDD_CMD_ENABLE_INT        0x10 // enable interrupts command
#define DCDD_CMD_DISABLE_INT       0x20 // disable interrupts command
#define DCDD_CMD_REDUCE_HEAD       0x40 // reduce head voltage command
#define DCDD_CMD_WRITE_ENABLE      0x80 // write enable command

#define PORT_DCDD_STATUS           0x08 // status port (in)
#define PORT_DCDD_SELECTOR         0x08 // selector port (out)
#define PORT_DCDD_SECTOR           0x09 // sector port (in)
#define PORT_DCDD_COMMAND          0x09 // command port (out)
#define PORT_DCDD_DATA             0x0A // data port (in/out)

#define DCDD_SELECTOR_DRV_SELECT   0x80 // ACTIVE_HIGH - disk controller - no disk selected
#define DCDD_SECTOR_TRUE           0x01 // ACTIVE_LOW  - the sector is positioned to r/w

#define track_pos(disk)  (DCDD_SECTORS_PER_TRACK * DCDD_BYTES_PER_SECTOR * disk.track)
#define sector_pos(disk) (DCDD_BYTES_PER_SECTOR * disk.sector)
#define head_pos(disk)   (track_pos(disk) + sector_pos(disk) + disk.index)

static uint8_t dcdd_status(DCDD* dcdd);
static uint8_t dcdd_sector(DCDD* dcdd);
static uint8_t dcdd_read(DCDD* dcdd);
static void dcdd_write(DCDD* dcdd, uint8_t value);
static void dcdd_selector(DCDD* dcdd, uint8_t value);
static void dcdd_command(DCDD* dcdd, uint8_t value);

int dcdd_init(DCDD* dcdd) {
	dcdd->disks = (DISK*)malloc(sizeof(DISK) * DCDD_MAX_DISKS);
	if (dcdd->disks == NULL) {
		return 1;
	}
	memset(dcdd->disks, 0, sizeof(DISK) * DCDD_MAX_DISKS);
	return 0;
}
void dcdd_free(DCDD* dcdd) {
	if (dcdd->disks != NULL) {
		for (int i = 0; i < DCDD_MAX_DISKS; ++i) {
			if (dcdd->disks[i].file != NULL) {
				fclose(dcdd->disks[i].file);
				dcdd->disks[i].file = NULL;
			}
		}
		free(dcdd->disks);
		dcdd->disks = NULL;
	}
}
void dcdd_reset(DCDD* dcdd) {
	dcdd->selector = DCDD_SELECTOR_DRV_SELECT;
	for (int i = 0; i < DCDD_MAX_DISKS; ++i) {
		dcdd->disks[i].track = 0;
		dcdd->disks[i].sector = 0;
		dcdd->disks[i].index = 0;
		dcdd->disks[i].status = 
			DCDD_STATUS_WRITE_READY |
			DCDD_STATUS_MOVE_HEAD   |
			DCDD_STATUS_HEAD_LOADED |
			DCDD_STATUS_DRV_SELECT  |
			DCDD_STATUS_INT_ENABLED |
			DCDD_STATUS_TRACK_ZERO  |
			DCDD_STATUS_READ_READY;
	}
}
int dcdd_read_io(DCDD* dcdd, uint8_t port, uint8_t* value) {
	switch (port) {		

		case PORT_DCDD_STATUS:
			*value = dcdd_status(dcdd);
			break;

		case PORT_DCDD_SECTOR:
			*value = dcdd_sector(dcdd);
			break;

		case PORT_DCDD_DATA:
			*value = dcdd_read(dcdd);
			break;

		default:
			return 0;
	}
	return 1;
}
int dcdd_write_io(DCDD* dcdd, uint8_t port, uint8_t value) {
	switch (port) {

		case PORT_DCDD_COMMAND:
			dcdd_command(dcdd, value);
			break;

		case PORT_DCDD_SELECTOR:
			dcdd_selector(dcdd, value);
			break;

		case PORT_DCDD_DATA:
			dcdd_write(dcdd, value);
			break;

		default:
			return 0;
	}
	return 1;
}

static uint8_t dcdd_status(DCDD* dcdd) {
	if (dcdd->selector & DCDD_SELECTOR_DRV_SELECT) {
		return 0xFF;
	}
	return dcdd->disks[dcdd->selector].status;
}
static uint8_t dcdd_sector(DCDD* dcdd) {
	if (dcdd->selector & DCDD_SELECTOR_DRV_SELECT) {
		return 0xFF;
	}

	if (dcdd->disks[dcdd->selector].status & DCDD_STATUS_HEAD_LOADED) {
		// head not loaded
		return 0xFF;
	}
	
	dcdd->disks[dcdd->selector].sector++;
	if (dcdd->disks[dcdd->selector].sector >= DCDD_SECTORS_PER_TRACK) {
		dcdd->disks[dcdd->selector].sector = 0;
	}
	dcdd->disks[dcdd->selector].index = 0;
	return (dcdd->disks[dcdd->selector].sector << 1);
}
static uint8_t dcdd_read(DCDD* dcdd) {
	if (dcdd->selector & DCDD_SELECTOR_DRV_SELECT) {
		return 0xFF;
	}

	if (dcdd->disks[dcdd->selector].status & DCDD_STATUS_HEAD_LOADED) {
		// head not loaded
		return 0xFF;
	}

	if (dcdd->disks[dcdd->selector].file == NULL) {
		return 0xFF;
	}

	uint32_t offset = head_pos(dcdd->disks[dcdd->selector]);
	if (fseek(dcdd->disks[dcdd->selector].file, offset, SEEK_SET) != 0) {
		return 0xFF;
	}
	
	uint8_t v = 0;
	fread(&v, 1, 1, dcdd->disks[dcdd->selector].file);
	dcdd->disks[dcdd->selector].index++;
	return v;
}
static void dcdd_write(DCDD* dcdd, uint8_t value) {
	if (dcdd->selector & DCDD_SELECTOR_DRV_SELECT) {
		return;
	}

	if (dcdd->disks[dcdd->selector].status & DCDD_STATUS_HEAD_LOADED) {
		// head not loaded
		return;
	}

	if (dcdd->disks[dcdd->selector].status & DCDD_STATUS_WRITE_READY) {
		// write protected
		return;
	}

	if (dcdd->disks[dcdd->selector].file == NULL) {
		return;
	}

	uint32_t offset = head_pos(dcdd->disks[dcdd->selector]);
	if (fseek(dcdd->disks[dcdd->selector].file, offset, SEEK_SET) != 0) {
		return;
	}
	
	fwrite(&value, 1, 1, dcdd->disks[dcdd->selector].file);
	dcdd->disks[dcdd->selector].index++;
}

static void dcdd_selector(DCDD* dcdd, uint8_t value) {
	if (value & DCDD_SELECTOR_DRV_SELECT) {
		/* deselect disk */
		if ((dcdd->selector & DCDD_SELECTOR_DRV_SELECT) == 0) {
			dcdd->disks[dcdd->selector].status |= DCDD_STATUS_DRV_SELECT | DCDD_STATUS_MOVE_HEAD;
			dcdd->selector = DCDD_SELECTOR_DRV_SELECT;
		}
	}
	else {
		uint8_t selector = value & 0x0F;
		if (dcdd->disks[selector].file == NULL) {
			/* disk error */
			dcdd->disks[selector].status |= DCDD_STATUS_DRV_SELECT | DCDD_STATUS_MOVE_HEAD;
			dcdd->selector = DCDD_SELECTOR_DRV_SELECT;
		}
		else {
			/* select disk */
			dcdd->disks[selector].status &= ~(DCDD_STATUS_DRV_SELECT | DCDD_STATUS_MOVE_HEAD);
			dcdd->selector = selector;
		}
	}
}

static void step_in(DCDD* dcdd) {
	if (dcdd->disks[dcdd->selector].track < DCDD_TRACKS_PER_DISK-1) {
		dcdd->disks[dcdd->selector].track++;
		dcdd->disks[dcdd->selector].sector = 0xFF;
		dcdd->disks[dcdd->selector].index = 0;
	}
	dcdd->disks[dcdd->selector].status |= DCDD_STATUS_TRACK_ZERO; // Track not 0
}
static void step_out(DCDD* dcdd) {
	if (dcdd->disks[dcdd->selector].track > 0) {
		dcdd->disks[dcdd->selector].track--;
		dcdd->disks[dcdd->selector].sector = 0xFF;
		dcdd->disks[dcdd->selector].index = 0;
	}
	else {
		dcdd->disks[dcdd->selector].status &= ~DCDD_STATUS_TRACK_ZERO; // Track 0
	}
}
static void load_head(DCDD* dcdd) {
	dcdd->disks[dcdd->selector].status &= ~DCDD_STATUS_HEAD_LOADED; // head loaded for r/w
	dcdd->disks[dcdd->selector].status &= ~DCDD_STATUS_READ_READY;  // read ready
	dcdd->disks[dcdd->selector].sector = 0xFF; // set sector to FF so next time it's read it will read 0.
}
static void unload_head(DCDD* dcdd) {
	dcdd->disks[dcdd->selector].status |= DCDD_STATUS_HEAD_LOADED; // head unloaded
	dcdd->disks[dcdd->selector].status |= DCDD_STATUS_READ_READY;  // read not ready
	dcdd->disks[dcdd->selector].status |= DCDD_STATUS_WRITE_READY; // write not ready
}
static void write_enable(DCDD* dcdd) {
	dcdd->disks[dcdd->selector].index = 0;
	dcdd->disks[dcdd->selector].status &= ~DCDD_STATUS_WRITE_READY;
}
static void enable_int(DCDD* dcdd) {
	dcdd->disks[dcdd->selector].status |= DCDD_STATUS_INT_ENABLED;
}
static void disable_int(DCDD* dcdd) {
	dcdd->disks[dcdd->selector].status &= ~DCDD_STATUS_INT_ENABLED;
}

static void dcdd_command(DCDD* dcdd, uint8_t value) {	

	if (dcdd->selector & DCDD_SELECTOR_DRV_SELECT) {
		return;
	}

	switch (value) {
		
		case DCDD_CMD_STEP_IN:
			step_in(dcdd);
			break;
		case DCDD_CMD_STEP_OUT:
			step_out(dcdd);
			break;
		case DCDD_CMD_LOAD_HEAD:
			load_head(dcdd);
			break;
		case DCDD_CMD_UNLOAD_HEAD:
			unload_head(dcdd);
			break;
		case DCDD_CMD_ENABLE_INT:
			enable_int(dcdd);
			break;
		case DCDD_CMD_DISABLE_INT:
			disable_int(dcdd);
			break;
		case DCDD_CMD_WRITE_ENABLE:
			write_enable(dcdd);
			break;
	}
}
