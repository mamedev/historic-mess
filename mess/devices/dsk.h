#ifndef DSK_H
#define DSK_H

#include "messdrv.h"

int dsk_floppy_load(int id, mame_file *fp, int open_mode);
void dsk_floppy_unload(int id);
void dsk_seek_callback(int drive, int physical_track);
void dsk_get_id_callback(int drive, struct chrn_id *id, int id_index, int side);
int dsk_get_sectors_per_track(int drive, int side);

void dsk_write_sector_data_from_buffer(int drive, int sector_index, int side, char *ptr, int length, int ddam);
void dsk_read_sector_data_into_buffer(int drive, int sector_index, int side, char *ptr, int length);

#define CONFIG_DEVICE_LEGACY_DSK(count) \
	CONFIG_DEVICE_LEGACY(IO_FLOPPY, (count), "dsk\0", DEVICE_LOAD_RESETS_NONE, OSD_FOPEN_RW_OR_READ, NULL, NULL, dsk_floppy_load, dsk_floppy_unload, floppy_status)


#endif /* DSK_H */