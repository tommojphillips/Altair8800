/* 88_dcdd.c
 * Altair Floppy Disk - 88-DCDD
 * Github: https:\\github.com\tommojphillips
 */

#include <stdint.h>
#include <stdio.h>

#include "88_dcdd.h"

 /*
			  7   6   5   4   3   2   1   0
			+---+---+---+---+---+---+---+---+
			|   |   |   Sector Number   | T |
			+---+---+---+---+---+---+---+---+

 * Sector number = sector number currently under the head, 0-31.
 * T = Sector True, is a 0 when the sector is positioned to read or write.
 * If head is unloaded, hardware returns 0xFF.
 */

//#define DCDD_DBG

#define DCDD_TRACKS_PER_DISK       77
#define DCDD_SECTORS_PER_TRACK     32
#define DCDD_BYTES_PER_SECTOR      137

#define DCDD_STATUS_WRITE_READY    0x01 // when 0, write device is ready
#define DCDD_STATUS_MOVE_HEAD      0x02 // when 0, head can be moved
#define DCDD_STATUS_HEAD_LOADED    0x04 // when 0, head is loaded for r/w
#define DCDD_STATUS_DRV_SELECT     0x08 // when 0, disk is selected and active
#define DCDD_STATUS_INT_ENABLED    0x20 // when 0, interrupts are enabled
#define DCDD_STATUS_TRACK_ZERO     0x40 // when 0, head is on track 0
#define DCDD_STATUS_READ_READY     0x80 // when 0, read device is ready

#define DCDD_CMD_STEP_IN           0x01 // step in head
#define DCDD_CMD_STEP_OUT          0x02 // step out head
#define DCDD_CMD_LOAD_HEAD         0x04 // load head
#define DCDD_CMD_UNLOAD_HEAD       0x08 // unload head
#define DCDD_CMD_ENABLE_INT        0x10 // enable interrupts
#define DCDD_CMD_DISABLE_INT       0x20 // disable interrupts
#define DCDD_CMD_REDUCE_HEAD       0x40 // reduce head voltage
#define DCDD_CMD_WRITE_ENABLE      0x80 // write enable

#define PORT_DCDD_STATUS   0x08 // 88-dcdd status port (in)
#define PORT_DCDD_SELECTOR 0x08 // 88-dcdd selector port (out)
#define PORT_DCDD_SECTOR   0x09 // 88-dcdd sector port (in)
#define PORT_DCDD_COMMAND  0x09 // 88-dcdd command port (out)
#define PORT_DCDD_DATA     0x0A // 88-dcdd data port (in/out)

#define dbg_err(x, ...) printf(x, __VA_ARGS__)

#ifdef DCDD_DBG
#define dbg_print(x, ...) printf(x, __VA_ARGS__)
#else
#define dbg_print(x, ...)
#endif

void dcdd_init(DCDD* dcdd) {
	dcdd->status = DCDD_STATUS_DRV_SELECT |  // Disk not selected
		           DCDD_STATUS_INT_ENABLED | // INT disabled
		           DCDD_STATUS_HEAD_LOADED;  // Head not loaded

	dcdd->selector = -1;
	dcdd->command = 0;
	dcdd->sector_pos = 0;
	dcdd->track_pos = 0;
}
int dcdd_read_io(DCDD* dcdd, uint8_t port, uint8_t* value) {
	switch (port) {		

		case PORT_DCDD_STATUS:
			*value = dcdd_status(dcdd);
			break;

		case PORT_DCDD_SECTOR:
			*value = dcdd_sector_pos(dcdd);
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

uint8_t dcdd_status(DCDD* dcdd) {
	dbg_print("dcdd status: %02X\n", dcdd->status);
	return dcdd->status;
}
uint8_t dcdd_sector_pos(DCDD* dcdd) {	
	uint8_t sector_reg = 0;
	if ((dcdd->status & DCDD_STATUS_HEAD_LOADED) == 0) {
		if (dcdd->sector_pos < DCDD_SECTORS_PER_TRACK-1) {
			dcdd->sector_pos++; 
			sector_reg = (dcdd->sector_pos << 1);
		}
		else {			
			dcdd->sector_pos = 0;
		}
		dcdd->read_index = 0;
	}
	else {
		sector_reg = 0xFF;
	}
	dbg_print("dcdd_sector_pos: reg=%02X, pos=%02X\n", sector_reg, dcdd->sector_pos);
	return sector_reg;
}
uint8_t dcdd_read(DCDD* dcdd) {
	uint32_t offset = ((dcdd->track_pos * DCDD_SECTORS_PER_TRACK) * DCDD_BYTES_PER_SECTOR) + (dcdd->sector_pos * DCDD_BYTES_PER_SECTOR) + dcdd->read_index;
	if (dcdd->selector & 0x80) {
		dbg_print("dcdd_read drive_unselected: %08X\n", offset);
		return 0;
	}
	else if (dcdd->selector > 0xF) {
		return 0;
	}
	else if (dcdd->disk_file[dcdd->selector] == NULL) {		
		return 0;
	}
	else if (fseek(dcdd->disk_file[dcdd->selector], offset, SEEK_SET) != 0) {
		return 0;
	}
	else {
		uint8_t v = 0;
		fread(&v, 1, 1, dcdd->disk_file[dcdd->selector]);
		dcdd->read_index++;
		dbg_print("dcdd_read: track: %02d, sector: %02d, %08X=%02X\n", dcdd->track_pos, dcdd->sector_pos, offset, v);
		return v;
	}
}
void dcdd_write(DCDD* dcdd, uint8_t value) {
	uint32_t offset = ((dcdd->track_pos * DCDD_SECTORS_PER_TRACK) * DCDD_BYTES_PER_SECTOR) + (dcdd->sector_pos * DCDD_BYTES_PER_SECTOR) + dcdd->read_index;
	if (dcdd->selector & 0x80) {
		dbg_print("dcdd_write drive_unselected: %08X\n", offset);
		return;
	}
	else if (dcdd->selector > 0xF) {
		return;
	}
	else if (dcdd->disk_file[dcdd->selector] == NULL) {
		return;
	}
	else if (fseek(dcdd->disk_file[dcdd->selector], offset, SEEK_SET) != 0) {
		return;
	}
	else {
		fwrite(&value, 1, 1, dcdd->disk_file[dcdd->selector]);
		dcdd->read_index++;
		dbg_print("dcdd_write: track: %02d, sector: %02d, %08X=%02X\n", dcdd->track_pos, dcdd->sector_pos, offset, v);
	}
}

void dcdd_selector(DCDD* dcdd, uint8_t value) {
	if (value & 0x80) {
		/* deselect drive */
		dcdd->selector = 0x80;
		dcdd->status |= DCDD_STATUS_DRV_SELECT;  // Disk is unselected and inactive
		dbg_print("dcdd selector: deselect drive\n");
	}
	else {
		if (dcdd->disk_file[value & 0xF] == NULL) {
			/* disk error */
			dcdd->selector = 0x80;
			dcdd->status &= ~DCDD_STATUS_DRV_SELECT; // Disk is selected and active
			dbg_err("dcdd selector: disk not in %c:\n", 'A' + (value & 0xF));
		}
		else {
			/* Select drive */
			dcdd->selector = value & 0xF;
			dcdd->status &= ~DCDD_STATUS_DRV_SELECT; // Disk is selected and active
			dbg_print("dcdd selector: %c\n", 'A' + dcdd->selector);
		}
	}
}

static void step_in(DCDD* dcdd) {
	if (dcdd->track_pos < DCDD_TRACKS_PER_DISK) {
		dcdd->track_pos++;
		dcdd->sector_pos = 0;
		dcdd->read_index = 0;
	}
	if (dcdd->track_pos > 0) 
		dcdd->status |= DCDD_STATUS_TRACK_ZERO;
	else
		dcdd->status &= ~DCDD_STATUS_TRACK_ZERO;
	dbg_print("dcdd command: step in, track=%d\n", dcdd->track_pos);
}
static void step_out(DCDD* dcdd) {
	if (dcdd->track_pos > 0) {
		dcdd->track_pos--;
		dcdd->sector_pos = 0;
		dcdd->read_index = 0;
	}
	if (dcdd->track_pos > 0)
		dcdd->status |= DCDD_STATUS_TRACK_ZERO;
	else
		dcdd->status &= ~DCDD_STATUS_TRACK_ZERO;
	dbg_print("dcdd command: step out, track=%d\n", dcdd->track_pos);
}
static void load_head(DCDD* dcdd) {
	dcdd->status &= ~DCDD_STATUS_HEAD_LOADED;
	dbg_print("dcdd command: load head\n");
}
static void unload_head(DCDD* dcdd) {
	dcdd->status |= DCDD_STATUS_HEAD_LOADED;
	dbg_print("dcdd command: unload head\n");
}
static void enable_int(DCDD* dcdd) {
	dcdd->status |= DCDD_STATUS_INT_ENABLED;
	dbg_print("dcdd command: enable int\n");
}
static void disable_int(DCDD* dcdd) {
	dcdd->status &= ~DCDD_STATUS_INT_ENABLED;
	dbg_print("dcdd command: disable int\n");
}
static void reduce_head(DCDD* dcdd) {
	dbg_print("dcdd command: reduce head\n");
}
static void write_enable(DCDD* dcdd) {
	dbg_print("dcdd command: write enable\n");
}

void dcdd_command(DCDD* dcdd, uint8_t value) {	
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
		case DCDD_CMD_REDUCE_HEAD:
			reduce_head(dcdd);
			break;
		case DCDD_CMD_WRITE_ENABLE:
			write_enable(dcdd);
			break;
		default:
			dbg_print("dcdd unk command: %02X\n", value);
			break;
	}
}

