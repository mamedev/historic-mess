/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include <stdarg.h>
#include "driver.h"
#include "cpu/z80/z80.h"
#include "mess/systems/trs80.h"

#define VERBOSE 0


UINT8 port_ff = 0;

static int trs80_load_cas = 0;

#define IRQ_TIMER       0x80
#define IRQ_FDC         0x40
static	UINT8			irq_status = 0;

#define MAX_LUMPS       192     /* crude storage units - don't now much about it */
#define MAX_GRANULES    8       /* lumps consisted of granules.. aha */
#define MAX_SECTORS     5       /* and granules of sectors */

/* this indicates whether the floppy images geometry shall be calculated */
static UINT8 first_fdc_access = 1;
static UINT8 motor_drive = 0;				/* currently running drive */
static short motor_count = 0;				/* time out for motor in frames */
static const char *floppy_name[4] = {0,};	/* filenames of the images */
static UINT8 tracks[4] = {0, }; 			/* total tracks count per drive */
static UINT8 heads[4] = {0, };				/* total heads count per drive */
static UINT8 spt[4] = {0, };				/* sector per track count per drive */
#if USE_TRACK
static UINT8 track[4] = {0, };				/* current track per drive */
#endif
static UINT8 head[4] = {0, };				/* current head per drive */
#if USE_SECTOR
static UINT8 sector[4] = {0, }; 			/* current sector per drive */
#endif
static short dir_sector[4] = {0, }; 		/* first directory sector (aka DDSL) */
static short dir_length[4] = {0, }; 		/* length of directory in sectors (aka DDGA) */

static const char *tape_name = NULL;

/* current tape file handles */
static void *tape_put_file = NULL;
static void *tape_get_file = NULL;

/* tape buffer for the first eight bytes at write (to extract a filename) */
static UINT8 tape_buffer[8];

static int tape_count = 0;      /* file offset within tape file */
static int put_bit_count = 0;   /* number of sync and data bits that were written */
static int get_bit_count = 0;   /* number of sync and data bits to read */
static int tape_bits = 0;       /* sync and data bits mask */
static int tape_time = 0;       /* time in cycles for the next bit at read */
static int in_sync = 0;         /* flag if writing to tape detected the sync header A5 already */
static int put_cycles = 0;      /* cycle count at last output port change */
static int get_cycles = 0;      /* cycle count at last input port read */

int trs80_videoram_r(int offset);
void trs80_videoram_w(int offset, int data);

static void tape_put_byte(UINT8 value);
static void tape_get_open(void);
static void tape_put_close(void);

#define FW TRS80_FONT_W
#define FH TRS80_FONT_H


void init_trs80(void)
{
	UINT8 *FNT = memory_region(REGION_GFX1);
    int i, y;

    for( i = 0x000; i < 0x080; i++ )
    {
		/* copy eight lines from the character generator */
        for (y = 0; y < 8; y++)
			FNT[i*FH+y] = FNT[0x0800+i*8+y] << 3;
		/* wipe out the lower lines (no descenders!) */
		for (y = 8; y < FH; y++)
			FNT[i*FH+y] = 0;
    }
	/* setup the 2x3 chunky block graphics (two times 64 characters) */
	for( i = 0x080; i < 0x100; i++ )
    {
        UINT8 b0, b1, b2, b3, b4, b5;
        b0 = (i & 0x01) ? 0xe0 : 0x00;
        b1 = (i & 0x02) ? 0x1c : 0x00;
        b2 = (i & 0x04) ? 0xe0 : 0x00;
        b3 = (i & 0x08) ? 0x1c : 0x00;
        b4 = (i & 0x10) ? 0xe0 : 0x00;
        b5 = (i & 0x20) ? 0x1c : 0x00;

		FNT[i*FH+ 0] = FNT[i*FH+ 1] = FNT[i*FH+ 2] = FNT[i*FH+ 3] = b0 | b1;
		FNT[i*FH+ 4] = FNT[i*FH+ 5] = FNT[i*FH+ 6] = FNT[i*FH+ 7] = b2 | b3;
		FNT[i*FH+ 8] = FNT[i*FH+ 9] = FNT[i*FH+10] = FNT[i*FH+11] = b4 | b5;
    }
}

static OPBASE_HANDLER( opbaseoverride )
{
	UINT8 *ram = memory_region(REGION_CPU1);
	if( trs80_load_cas && ram[0x3c00+3*64] == 0x3e )
    {
		trs80_load_cas = 0;
		if( tape_name )
		{
			UINT8 *buff = malloc(65536), *s;
			unsigned size, entry = 0, block_len, block_ofs = 0;
			UINT8 data;
			void *cmd;

			if( !buff )
			{
				logerror("failed to allocate 64K buffer\n");
				return 1;
			}
			cmd = image_fopen(IO_SNAPSHOT, 0, OSD_FILETYPE_IMAGE_RW, OSD_FOPEN_READ);
			if( !cmd )
				cmd = image_fopen(IO_CASSETTE, 0, OSD_FILETYPE_IMAGE_RW, OSD_FOPEN_READ);
			if( !cmd )
			{
				logerror("failed to open '%s'\n", tape_name);
			}
			else
			{
				size = osd_fread(cmd, buff, 65536);
				s = buff;
				while( size > 3 )
				{
					data = *s++;
					switch( data )
					{
					case 0x01:		   /* CMD file header */
					case 0x07:		   /* another type of CMD file header */
						case 0x3c:		   /* CAS file header */
						block_len = *s++;
						/* on CMD files size zero means size 256 */
						if( block_len == 0 )
							block_len += 256;
						block_ofs = *s++;
						block_ofs += 256 * *s++;
							if( data != 0x3c )
						{
							block_len -= 2;
							if( block_len == 0 )
								block_len = 256;
						}
						size -= 4;
						logerror("trs80_cmd_load block ($%02X) %d at $%04X\n", data, block_len, block_ofs);
						while( block_len && size )
							{
							cpu_writemem16(block_ofs, *s);
							s++;
							block_ofs++;
							block_len--;
							size--;
						}
						if( data == 0x3c )
							s++;
						break;
						case 0x02:
						block_len = *s++;
						size -= 1;
					case 0x78:
						entry = *s++;
							entry += 256 * *s++;
						logerror("trs80_cmd_load entry ($%02X) at $%04X\n", data, entry);
						size -= 3;
						if( size <= 3 )
							logerror("starting program at $%04X\n", block_ofs);
						break;
					default:
						size--;
					}
				}
				osd_fclose(cmd);
				cpu_set_pc(entry);
			}
			free(buff);
		}
        cpu_setOPbaseoverride(0,NULL);
    }
	return address;
}

int trs80_cassette_init(int id)
{
	tape_name = device_filename(IO_CASSETTE,id);
	return 0;
}

int trs80_floppy_init(int id)
{
	floppy_name[id] = device_filename(IO_FLOPPY,id);
	return 0;
}

void trs80_floppy_exit(int id)
{
	wd179x_stop_drive();
    floppy_name[id] = NULL;
    first_fdc_access = 1;
}

void trs80_init_machine(void)
{
	if( floppy_name[0] )
		wd179x_init(1);
	else
		wd179x_init(0);

    first_fdc_access = 1;
	if( tape_name )
	{
		trs80_load_cas = 1;
		cpu_setOPbaseoverride(0, opbaseoverride);
	}
}

void trs80_shutdown_machine(void)
{
	tape_put_close();
	wd179x_stop_drive();
}

int trs80_rom_load(int id)
{
	const char *name = device_filename(IO_CARTSLOT,id);
	if( name && name[0] )
	{
		/* copy rom name to cassette name (same loader) */
		tape_name = name;
		trs80_load_cas = 1;
		cpu_setOPbaseoverride(0, opbaseoverride);
    }
	return 0;
}

/*************************************
 *
 *              Tape emulation.
 *
 *************************************/

static void tape_put_byte(UINT8 value)
{
	if (tape_count < 8)
	{
		tape_buffer[tape_count++] = value;
		if (tape_count == 8)
		{
			/* BASIC tape ? */
			if (tape_buffer[1] == 0xd3)
			{
				char filename[12+1];
				UINT8 zeroes[256] = {0,};

                sprintf(filename, "basic%c.cas", tape_buffer[4]);
				tape_put_file = osd_fopen(Machine->gamedrv->name, filename, OSD_FILETYPE_IMAGE_RW, OSD_FOPEN_RW);
				osd_fwrite(tape_put_file, zeroes, 256);
				osd_fwrite(tape_put_file, tape_buffer, 8);
			}
			else
			/* SYSTEM tape ? */
			if (tape_buffer[1] == 0x55)
			{
				char filename[12+1];
				UINT8 zeroes[256] = {0,};

                sprintf(filename, "%-6.6s.cas", tape_buffer+2);
				tape_put_file = osd_fopen(Machine->gamedrv->name, filename, OSD_FILETYPE_IMAGE_RW, OSD_FOPEN_RW);
				osd_fwrite(tape_put_file, zeroes, 256);
				osd_fwrite(tape_put_file, tape_buffer, 8);
			}
		}
	}
	else
	{
		if (tape_put_file)
			osd_fwrite(tape_put_file, &value, 1);
	}
}

static void tape_put_close(void)
{
	/* file open ? */
	if (tape_put_file)
	{
		if (put_bit_count)
		{
			UINT8	value;
			while (put_bit_count < 16)
			{
					tape_bits <<= 1;
					put_bit_count++;
			}
			value = 0;
			if (tape_bits & 0x8000) value |= 0x80;
			if (tape_bits & 0x2000) value |= 0x40;
			if (tape_bits & 0x0800) value |= 0x20;
			if (tape_bits & 0x0200) value |= 0x10;
			if (tape_bits & 0x0080) value |= 0x08;
			if (tape_bits & 0x0020) value |= 0x04;
			if (tape_bits & 0x0008) value |= 0x02;
			if (tape_bits & 0x0002) value |= 0x01;
			tape_put_byte(value);
		}
		osd_fclose(tape_put_file);
	}
	tape_count = 0;
	tape_put_file = 0;
}

static void tape_get_byte(void)
{
	int 	count;
	UINT8	value;
	if (tape_get_file)
	{
		count = osd_fread(tape_get_file, &value, 1);
		if (count == 0)
		{
				value = 0;
				osd_fclose(tape_get_file);
				tape_get_file = 0;
		}
		tape_bits |= 0xaaaa;
		if (value & 0x80) tape_bits ^= 0x4000;
		if (value & 0x40) tape_bits ^= 0x1000;
		if (value & 0x20) tape_bits ^= 0x0400;
		if (value & 0x10) tape_bits ^= 0x0100;
		if (value & 0x08) tape_bits ^= 0x0040;
		if (value & 0x04) tape_bits ^= 0x0010;
		if (value & 0x02) tape_bits ^= 0x0004;
		if (value & 0x01) tape_bits ^= 0x0001;
		get_bit_count = 16;
		tape_count++;
	}
}

static void tape_get_open(void)
{
	/* TODO: remove this */
	unsigned char *RAM = memory_region(REGION_CPU1);

	if (!tape_get_file)
	{
        char filename[12+1];

        sprintf(filename, "%-6.6s.cas", RAM + 0x41e8);
		logerror("filename %s\n", filename);
		tape_get_file = osd_fopen(Machine->gamedrv->name, filename, OSD_FILETYPE_IMAGE_RW, OSD_FOPEN_READ);
		tape_count = 0;
	}
}

/*************************************
 *
 *              Port handlers.
 *
 *************************************/

void trs80_port_ff_w(int offset, int data)
{
	int changes = port_ff ^ data;

	/* tape output changed ? */
	if( changes & 0x03 )
	{
		/* virtual tape ? */
		if( readinputport(0) & 0x20 )
		{
			int now_cycles = cpu_gettotalcycles();
			int diff = now_cycles - put_cycles;
			UINT8 value;
			/* overrun since last write ? */
			if (diff > 4000)
			{
				/* reset tape output */
				tape_put_close();
				put_bit_count = tape_bits = in_sync = 0;
			}
			else
			/* just wrote the interesting value ? */
			if( (data & 0x03) == 0x01 )
			{
				/* change within time for a 1 bit ? */
				if( diff < 2000 )
				{
					tape_bits = (tape_bits << 1) | 1;
					/* in sync already ? */
					if( in_sync )
					{
						/* count 1 bit */
						put_bit_count += 1;
					}
					else
					{
						/* try to find sync header A5 */
						if( tape_bits == 0xcc33 )
						{
							in_sync = 1;
							put_bit_count = 16;
						}
					}
				}
				else	/* no change indicates a 0 bit */
				{
					/* shift twice */
					tape_bits <<= 2;
					/* in sync already ? */
					if( in_sync )
						put_bit_count += 2;
				}

				/* collected 8 sync plus 8 data bits ? */
				if( put_bit_count >= 16 )
				{
					/* extract data bits to value */
					value = 0;
					if (tape_bits & 0x8000) value |= 0x80;
					if (tape_bits & 0x2000) value |= 0x40;
					if (tape_bits & 0x0800) value |= 0x20;
					if (tape_bits & 0x0200) value |= 0x10;
					if (tape_bits & 0x0080) value |= 0x08;
					if (tape_bits & 0x0020) value |= 0x04;
					if (tape_bits & 0x0008) value |= 0x02;
					if (tape_bits & 0x0002) value |= 0x01;
					put_bit_count -= 16;
					tape_bits = 0;
					tape_put_byte(value);
				}
			}
			/* remember the cycle count of this write */
			put_cycles = now_cycles;
		}
		else
		{
			switch( data & 0x03 )
			{
			case 0: /* 0.46 volts */
				speaker_level_w(0,1);
				break;
			case 1:
			case 3: /* 0.00 volts */
				speaker_level_w(0,0);
				break;
			case 2: /* 0.85 volts */
				speaker_level_w(0,2);
				break;
			}
		}
	}

	/* font width change ? (32<->64 characters per line) */
	if( changes & 0x08 )
		memset(dirtybuffer, 1, 64 * 16);

	port_ff = data;
}

int trs80_port_ff_r(int offset)
{
	int now_cycles = cpu_gettotalcycles();
	/* virtual tape ? */
	if( readinputport(0) & 0x20 )
	{
        int     diff = now_cycles - get_cycles;
		/* overrun since last read ? */
		if (diff >= 4000)
		{
			if (tape_get_file)
			{
				osd_fclose(tape_get_file);
				tape_get_file = 0;
			}
			get_bit_count = tape_bits = tape_time = 0;
		}
		else /* check what he will get for input */
		{
			/* count down cycles */
			tape_time -= diff;
			/* time for the next sync or data bit ? */
			if (tape_time <= 0)
			{
				/* approx. time for a bit */
				tape_time += 1570;
				/* need to read get new data ? */
				if (--get_bit_count <= 0)
				{
					tape_get_open();
					tape_get_byte();
				}
				/* shift next sync or data bit to bit 16 */
				tape_bits <<= 1;
				/* if bit is set, set port_ff bit 4
				   which is then read as bit 7 */
				if (tape_bits & 0x10000)
					port_ff |= 0x80;
			}
		}
		/* remember the cycle count of this read */
		get_cycles = now_cycles;
	}
	return (port_ff << 3) & 0xc0;
}

int trs80_port_xx_r(int offset)
{
        return 0xff;
}

/*************************************
 *
 *      Interrupt handlers.
 *
 *************************************/

int trs80_timer_interrupt(void)
{
	if( (irq_status & IRQ_TIMER) == 0 )
	{
		irq_status |= IRQ_TIMER;
		cpu_set_irq_line (0, 0, HOLD_LINE);
		return 0;
	}
	return ignore_interrupt ();
}

int trs80_fdc_interrupt(void)
{
	if ((irq_status & IRQ_FDC) == 0)
	{
		irq_status |= IRQ_FDC;
		cpu_set_irq_line (0, 0, HOLD_LINE);
        return 0;
	}
	return ignore_interrupt ();
}

void trs80_fdc_callback(int event)
{
	switch (event)
	{
		case WD179X_IRQ_CLR:
			irq_status &= ~IRQ_FDC;
			break;
		case WD179X_IRQ_SET:
			trs80_fdc_interrupt();
			break;
	}
}

int trs80_frame_interrupt (void)
{
	if (motor_count && !--motor_count)
		wd179x_stop_drive();
	return 0;
}

void trs80_nmi_generate (int param)
{
	cpu_cause_interrupt (0, Z80_NMI_INT);
}

/*************************************
 *                                   *
 *      Memory handlers              *
 *                                   *
 *************************************/

int trs80_printer_r(int offset)
{
	/* nothing yet :( */
	return 0;
}

void trs80_printer_w(int offset, int data)
{
	/* nothing yet :( */
}

int trs80_status_r(int offset)
{
	/* If the floppy isn't emulated, return 0 */
	if ((readinputport (0) & 0x80) == 0) return 0;
        return wd179x_status_r(offset);
}

int trs80_track_r(int offset)
{
	return wd179x_track_r(offset);
}

int trs80_sector_r(int offset)
{
	return wd179x_sector_r(offset);
}

int trs80_data_r(int offset)
{
	return wd179x_data_r(offset);
}

void trs80_command_w(int offset, int data)
{
	wd179x_command_w(offset, data);
}

void trs80_track_w(int offset, int data)
{
	wd179x_track_w(offset, data);
}

void trs80_sector_w(int offset, int data)
{
	wd179x_sector_w(offset, data);
}

void trs80_data_w(int offset, int data)
{
	wd179x_data_w(offset, data);
}

int trs80_irq_status_r(int offset)
{
int result = irq_status;
	irq_status &= ~(IRQ_TIMER | IRQ_FDC);
	return result;
}

void trs80_irq_mask_w(int offset, int data)
{
/*	irq_mask = data; */
}

void trs80_motor_w(int offset, int data)
{
	UINT8 buff[16];
	UINT8 drive = 255;
	int n;
	void *file0, *file1;

	logerror("trs80 motor_w $%02X\n", data);

	switch (data)
	{
		case 1:
			drive = 0;
			head[drive] = 0;
			break;
		case 2:
			drive = 1;
			head[drive] = 0;
			break;
		case 4:
			drive = 2;
			head[drive] = 0;
			break;
		case 8:
			drive = 3;
			head[drive] = 0;
			break;
		case 9:
			drive = 0;
			head[drive] = 1;
			break;
		case 10:
			drive = 1;
			head[drive] = 1;
			break;
		case 12:
			drive = 2;
			head[drive] = 1;
			break;
	}

	if (drive > 3)
		return;

	/* no image name given for that drive ? */
	if (!floppy_name[drive])
		return;

	/* currently selected drive */
	motor_drive = drive;

	/* let it run about 5 seconds */
	motor_count = 5 * 60;

	/* select the drive / head */
	file0 = wd179x_select_drive(drive, head[drive], trs80_fdc_callback, floppy_name[drive]);

	if (!file0)
		return;
	/* first drive selected first time ? */
	if (!first_fdc_access)
		return;

	first_fdc_access = 0;

	if (file0 == REAL_FDD)
		return;

	/* read first bytes from boot sector */
	for (n = 0; n < 4; n++)
	{
		file1 = wd179x_select_drive(n, head[n], trs80_fdc_callback, floppy_name[n]);
		if (!file1)
			continue;
		if (file1 == REAL_FDD)
		{
			osd_fseek(file0, 2 * 256 + n * 16, SEEK_SET);
			osd_fread(file0, buff, 16);
			tracks[n] = buff[3] + 1;
			heads[n] = (buff[7] & 0x40) ? 2 : 1;
			spt[n] = buff[4] / heads[n];
			dir_sector[n] = 5 * buff[0] * buff[5];
			dir_length[n] = 5 * buff[9];
		}
		else
		{
			osd_fseek(file1, 0, SEEK_SET);
			osd_fread(file1, buff, 2);
			if (buff[0] != 0x00 || buff[1] != 0xfe)
			{
				wd179x_read_sectormap(n, &tracks[n], &heads[n], &spt[n]);
			}
			else
			{
				osd_fseek(file0, 2 * 256 + n * 16, SEEK_SET);
				osd_fread(file0, buff, 16);
				tracks[n] = buff[3] + 1;
				heads[n] = (buff[7] & 0x40) ? 2 : 1;
				spt[n] = buff[4] / heads[n];
				dir_sector[n] = 5 * buff[0] * buff[5];
				dir_length[n] = 5 * buff[9];
			}
        }
		wd179x_set_geometry(n, tracks[n], heads[n], spt[n], 256, dir_sector[n], dir_length[n], 0);
	}
	wd179x_select_drive(drive, head[drive], trs80_fdc_callback, floppy_name[drive]);
}

/*************************************
 *      Keyboard                     *
 *************************************/
int trs80_keyboard_r(int offset)
{
	int result = 0;

    if( setup_active() || onscrd_active() )
		return result;

    if (offset & 1)
		result |= input_port_1_r(0);
	if (offset & 2)
		result |= input_port_2_r(0);
	if (offset & 4)
		result |= input_port_3_r(0);
	if (offset & 8)
		result |= input_port_4_r(0);
	if (offset & 16)
		result |= input_port_5_r(0);
	if (offset & 32)
		result |= input_port_6_r(0);
	if (offset & 64)
		result |= input_port_7_r(0);
	if (offset & 128)
		result |= input_port_8_r(0);

	return result;
}

/*************************************
 *      Video RAM                    *
 *************************************/

void trs80_videoram_w(int offset, int data)
{
	if (data == videoram[offset])
		return; 		/* no change */

	videoram[offset] = data;
	dirtybuffer[offset] = 1;
}


