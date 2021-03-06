/***************************************************************************

	commodore c65 home computer
	PeT mess@utanet.at

    documention
     www.funet.fi

***************************************************************************/

#include "driver.h"
#include "sound/sid6581.h"
#include "machine/6526cia.h"

#define VERBOSE_DBG 0
#include "includes/cbm.h"
#include "includes/vic4567.h"
#include "includes/cbmserb.h"
#include "includes/vc1541.h"

#include "includes/c65.h"

static ADDRESS_MAP_START( c65_mem , ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x00000, 0x07fff) AM_RAMBANK(11)
	AM_RANGE(0x08000, 0x09fff) AM_READWRITE(MRA8_BANK1, MWA8_BANK12)
	AM_RANGE(0x0a000, 0x0bfff) AM_READWRITE(MRA8_BANK2, MWA8_BANK13)
	AM_RANGE(0x0c000, 0x0cfff) AM_READWRITE(MRA8_BANK3, MWA8_BANK14)
	AM_RANGE(0x0d000, 0x0d7ff) AM_READWRITE(MRA8_BANK4, MWA8_BANK5)
	AM_RANGE(0x0d800, 0x0dbff) AM_READWRITE(MRA8_BANK6, MWA8_BANK7)
	AM_RANGE(0x0dc00, 0x0dfff) AM_READWRITE(MRA8_BANK8, MWA8_BANK9)
	AM_RANGE(0x0e000, 0x0ffff) AM_READWRITE(MRA8_BANK10, MWA8_BANK15)
	AM_RANGE(0x10000, 0x1f7ff) AM_RAM
	AM_RANGE(0x1f800, 0x1ffff) AM_RAM AM_BASE( &c64_colorram)

	AM_RANGE(0x20000, 0x23fff) AM_ROM /* &c65_dos,	   maps to 0x8000    */
	AM_RANGE(0x24000, 0x28fff) AM_ROM /* reserved */
	AM_RANGE(0x29000, 0x29fff) AM_ROM AM_BASE( &c65_chargen)
	AM_RANGE(0x2a000, 0x2bfff) AM_ROM AM_BASE( &c64_basic)
	AM_RANGE(0x2c000, 0x2cfff) AM_ROM AM_BASE( &c65_interface)
	AM_RANGE(0x2d000, 0x2dfff) AM_ROM AM_BASE( &c64_chargen)
	AM_RANGE(0x2e000, 0x2ffff) AM_ROM AM_BASE( &c64_kernal)

	AM_RANGE(0x30000, 0x31fff) AM_ROM /*&c65_monitor,	  monitor maps to 0x6000    */
	AM_RANGE(0x32000, 0x37fff) AM_ROM /*&c65_basic, */
	AM_RANGE(0x38000, 0x3bfff) AM_ROM /*&c65_graphics, */
	AM_RANGE(0x3c000, 0x3dfff) AM_ROM /* reserved */
	AM_RANGE(0x3e000, 0x3ffff) AM_ROM /* &c65_kernal, */

	AM_RANGE(0x40000, 0x7ffff) AM_NOP
	/* 8 megabyte full address space! */
ADDRESS_MAP_END

#define DIPS_HELPER(bit, name, keycode) \
   PORT_BIT(bit, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(name) PORT_CODE(keycode)

#define C65_DIPS \
     PORT_START \
	 DIPS_HELPER( 0x8000, "Quickload", KEYCODE_SLASH_PAD)\
	 PORT_BIT (0x7c00, 0x0, IPT_UNUSED) /* no tape */\
	PORT_DIPNAME   ( 0x80, 0x80, "Sid Chip Type")\
	PORT_DIPSETTING(  0, "MOS6581" )\
	PORT_DIPSETTING(0x80, "MOS8580" )\
	 /*PORT_DIPNAME (0x1c, 0x00, "Cartridge Type")*/\
	 /*PORT_DIPSETTING (0, "Automatic")*/\
	 /*PORT_DIPSETTING (4, "Ultimax (GAME)")*/\
	 /*PORT_DIPSETTING (8, "C64 (EXROM)")*/\
	 /*PORT_DIPSETTING (0x10, "CBM Supergames")*/\
	 /*PORT_DIPSETTING (0x14, "Ocean Robocop2")*/\
	 PORT_DIPNAME (0x02, 0x02, "Serial Bus/Device 10")\
	 PORT_DIPSETTING (0, DEF_STR( None ))\
	 PORT_DIPSETTING (2, "Floppy Drive Simulation")\
	 PORT_DIPNAME (0x01, 0x01, "Serial Bus/Device 11")\
	 PORT_DIPSETTING (0, DEF_STR( None ))\
	 PORT_DIPSETTING (1, "Floppy Drive Simulation")


INPUT_PORTS_START (c65)
	C64_DIPS
	C65_DIPS
	PORT_START
	DIPS_HELPER (0x8000, "Arrow-Left", KEYCODE_TILDE)
	DIPS_HELPER (0x4000, "1 !   BLK   ORNG", KEYCODE_1)
	DIPS_HELPER (0x2000, "2 \"   WHT   BRN", KEYCODE_2)
	DIPS_HELPER (0x1000, "3 #   RED   L RED", KEYCODE_3)
	DIPS_HELPER (0x0800, "4 $   CYN   D GREY", KEYCODE_4)
	DIPS_HELPER (0x0400, "5 %   PUR   GREY", KEYCODE_5)
	DIPS_HELPER (0x0200, "6 &   GRN   L GRN", KEYCODE_6)
	DIPS_HELPER (0x0100, "7 '   BLU   L BLU", KEYCODE_7)
	DIPS_HELPER (0x0080, "8 (   YEL   L GREY", KEYCODE_8)
	DIPS_HELPER (0x0040, "9 )   RVS-ON", KEYCODE_9)
	DIPS_HELPER (0x0020, "0     RVS-OFF", KEYCODE_0)
	DIPS_HELPER (0x0010, "+", KEYCODE_PLUS_PAD)
	DIPS_HELPER (0x0008, "-", KEYCODE_MINUS_PAD)
	DIPS_HELPER (0x0004, "Pound", KEYCODE_MINUS)
	DIPS_HELPER (0x0002, "HOME CLR", KEYCODE_EQUALS)
	DIPS_HELPER (0x0001, "DEL INST", KEYCODE_BACKSPACE)
	PORT_START
	DIPS_HELPER (0x8000, "(C65)TAB", KEYCODE_TAB)
	DIPS_HELPER (0x4000, "Q", KEYCODE_Q)
	DIPS_HELPER (0x2000, "W", KEYCODE_W)
	DIPS_HELPER (0x1000, "E", KEYCODE_E)
	DIPS_HELPER (0x0800, "R", KEYCODE_R)
	DIPS_HELPER (0x0400, "T", KEYCODE_T)
	DIPS_HELPER (0x0200, "Y", KEYCODE_Y)
	DIPS_HELPER (0x0100, "U", KEYCODE_U)
	DIPS_HELPER (0x0080, "I", KEYCODE_I)
	DIPS_HELPER (0x0040, "O", KEYCODE_O)
	DIPS_HELPER (0x0020, "P", KEYCODE_P)
	DIPS_HELPER (0x0010, "At", KEYCODE_OPENBRACE)
	DIPS_HELPER (0x0008, "*", KEYCODE_ASTERISK)
	DIPS_HELPER (0x0004, "Arrow-Up Pi", KEYCODE_CLOSEBRACE)
	DIPS_HELPER (0x0002, "RESTORE", KEYCODE_PRTSCR)
	DIPS_HELPER (0x0001, "CTRL", KEYCODE_RCONTROL)
	PORT_START
	PORT_DIPNAME (0x8000, 0, "(Left-Shift) SHIFT-LOCK (switch)") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_DIPSETTING(  0, DEF_STR(Off) )
	PORT_DIPSETTING(0x8000, DEF_STR(On) )
	DIPS_HELPER (0x4000, "A", KEYCODE_A)
	DIPS_HELPER (0x2000, "S", KEYCODE_S)
	DIPS_HELPER (0x1000, "D", KEYCODE_D)
	DIPS_HELPER (0x0800, "F", KEYCODE_F)
	DIPS_HELPER (0x0400, "G", KEYCODE_G)
	DIPS_HELPER (0x0200, "H", KEYCODE_H)
	DIPS_HELPER (0x0100, "J", KEYCODE_J)
	DIPS_HELPER (0x0080, "K", KEYCODE_K)
	DIPS_HELPER (0x0040, "L", KEYCODE_L)
	DIPS_HELPER (0x0020, ": [", KEYCODE_COLON)
	DIPS_HELPER (0x0010, "; ]", KEYCODE_QUOTE)
	DIPS_HELPER (0x0008, "=", KEYCODE_BACKSLASH)
	DIPS_HELPER (0x0004, "RETURN", KEYCODE_ENTER)
	DIPS_HELPER (0x0002, "CBM", KEYCODE_RALT)
	DIPS_HELPER (0x0001, "Left-Shift", KEYCODE_LSHIFT)
	PORT_START
	DIPS_HELPER (0x8000, "Z", KEYCODE_Z)
	DIPS_HELPER (0x4000, "X", KEYCODE_X)
	DIPS_HELPER (0x2000, "C", KEYCODE_C)
	DIPS_HELPER (0x1000, "V", KEYCODE_V)
	DIPS_HELPER (0x0800, "B", KEYCODE_B)
	DIPS_HELPER (0x0400, "N", KEYCODE_N)
	DIPS_HELPER (0x0200, "M", KEYCODE_M)
	DIPS_HELPER (0x0100, ", <", KEYCODE_COMMA)
	DIPS_HELPER (0x0080, ". >", KEYCODE_STOP)
	DIPS_HELPER (0x0040, "/ ?", KEYCODE_SLASH)
	DIPS_HELPER (0x0020, "Right-Shift", KEYCODE_RSHIFT)
	DIPS_HELPER (0x0010, "(Right-Shift Cursor-Down)CRSR-UP",
				 KEYCODE_8_PAD)
	DIPS_HELPER (0x0008, "Space", KEYCODE_SPACE)
	DIPS_HELPER (0x0004, "(Right-Shift Cursor-Right)CRSR-LEFT",
				 KEYCODE_4_PAD)
	DIPS_HELPER (0x0002, "CRSR-DOWN", KEYCODE_2_PAD)
	DIPS_HELPER (0x0001, "CRSR-RIGHT", KEYCODE_6_PAD)
	PORT_START
	DIPS_HELPER (0x8000, "STOP RUN", KEYCODE_ESC)
	DIPS_HELPER (0x4000, "(C65)ESC", KEYCODE_F1)
	DIPS_HELPER (0x2000, "(C65)ALT", KEYCODE_F2)
	PORT_DIPNAME (0x1000, 0, "(C65)CAPSLOCK(switch)") PORT_CODE(KEYCODE_F3)
	PORT_DIPSETTING(  0, DEF_STR(Off) )
	PORT_DIPSETTING(0x1000, DEF_STR(On) )
	DIPS_HELPER (0x0800, "(C65)NO SCRL", KEYCODE_F4)
	DIPS_HELPER (0x0400, "f1 f2", KEYCODE_F5)
	DIPS_HELPER (0x0200, "f3 f4", KEYCODE_F6)
	DIPS_HELPER (0x0100, "f5 f6", KEYCODE_F7)
	DIPS_HELPER (0x0080, "f7 f8", KEYCODE_F8)
	DIPS_HELPER (0x0040, "(C65)f9 f10", KEYCODE_F9)
	DIPS_HELPER (0x0020, "(C65)f11 f12", KEYCODE_F10)
	DIPS_HELPER (0x0010, "(C65)f13 f14", KEYCODE_F11)
	DIPS_HELPER (0x0008, "(C65)HELP", KEYCODE_F12)
INPUT_PORTS_END

INPUT_PORTS_START (c65ger)
	 C64_DIPS
     C65_DIPS
	 PORT_START
     DIPS_HELPER (0x8000, "_                    < >", KEYCODE_TILDE)
	 DIPS_HELPER (0x4000, "1 !  BLK   ORNG", KEYCODE_1)
	 DIPS_HELPER (0x2000, "2 \"  WHT   BRN", KEYCODE_2)
	 DIPS_HELPER (0x1000, "3 #  RED   L RED       Paragraph", KEYCODE_3)
	 DIPS_HELPER (0x0800, "4 $  CYN   D GREY", KEYCODE_4)
	 DIPS_HELPER (0x0400, "5 %  PUR   GREY", KEYCODE_5)
	 DIPS_HELPER (0x0200, "6 &  GRN   L GRN", KEYCODE_6)
	 DIPS_HELPER (0x0100, "7 '  BLU   L BLU	      /", KEYCODE_7)
	 DIPS_HELPER (0x0080, "8 (  YEL   L GREY", KEYCODE_8)
	 DIPS_HELPER (0x0040, "9 )  RVS-ON", KEYCODE_9)
	 DIPS_HELPER (0x0020, "0    RVS-OFF           =", KEYCODE_0)
	 DIPS_HELPER (0x0010, "+                    Sharp-S ?", KEYCODE_PLUS_PAD)
	 DIPS_HELPER (0x0008, "-                    '  `",KEYCODE_MINUS_PAD)
	 DIPS_HELPER (0x0004, "\\                    [ Arrow-Up", KEYCODE_MINUS)
	 DIPS_HELPER (0x0002, "HOME CLR", KEYCODE_EQUALS)
	 DIPS_HELPER (0x0001, "DEL INST", KEYCODE_BACKSPACE)
	 PORT_START
     DIPS_HELPER (0x8000, "(C65)TAB", KEYCODE_TAB)
	 DIPS_HELPER (0x4000, "Q", KEYCODE_Q)
	 DIPS_HELPER (0x2000, "W", KEYCODE_W)
	 DIPS_HELPER (0x1000, "E", KEYCODE_E)
	 DIPS_HELPER (0x0800, "R", KEYCODE_R)
	 DIPS_HELPER (0x0400, "T", KEYCODE_T)
	 DIPS_HELPER (0x0200, "Y                    Z", KEYCODE_Y)
	 DIPS_HELPER (0x0100, "U", KEYCODE_U)
	 DIPS_HELPER (0x0080, "I", KEYCODE_I)
	 DIPS_HELPER (0x0040, "O", KEYCODE_O)
	 DIPS_HELPER (0x0020, "P", KEYCODE_P)
	 DIPS_HELPER (0x0010, "Paragraph Arrow-Up   Diaresis-U",KEYCODE_OPENBRACE)
	 DIPS_HELPER (0x0008, "* `                  + *",KEYCODE_ASTERISK)
	 DIPS_HELPER (0x0004, "Sum Pi               ] \\",KEYCODE_CLOSEBRACE)
	 DIPS_HELPER (0x0002, "RESTORE", KEYCODE_PRTSCR)
	 DIPS_HELPER (0x0001, "CTRL", KEYCODE_RCONTROL)
	 PORT_START
     PORT_DIPNAME (0x8000, 0, "(Left-Shift)SHIFT-LOCK (switch)") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_DIPSETTING(  0, DEF_STR(Off) )\
	PORT_DIPSETTING(0x8000, DEF_STR(On) )\
	 DIPS_HELPER (0x4000, "A", KEYCODE_A)
	 DIPS_HELPER (0x2000, "S", KEYCODE_S)
	 DIPS_HELPER (0x1000, "D", KEYCODE_D)
	 DIPS_HELPER (0x0800, "F", KEYCODE_F)
	 DIPS_HELPER (0x0400, "G", KEYCODE_G)
	 DIPS_HELPER (0x0200, "H", KEYCODE_H)
	 DIPS_HELPER (0x0100, "J", KEYCODE_J)
	 DIPS_HELPER (0x0080, "K", KEYCODE_K)
	 DIPS_HELPER (0x0040, "L", KEYCODE_L)
	 DIPS_HELPER (0x0020, ": [                  Diaresis-O",
				  KEYCODE_COLON)
	 DIPS_HELPER (0x0010, "; ]                  Diaresis-A",
				  KEYCODE_QUOTE)
	 DIPS_HELPER (0x0008, "=                    # '",
				  KEYCODE_BACKSLASH)
	 DIPS_HELPER (0x0004, "RETURN", KEYCODE_ENTER)
	 DIPS_HELPER (0x0002, "CBM", KEYCODE_RALT)
	 DIPS_HELPER (0x0001, "Left-Shift", KEYCODE_LSHIFT)
	 PORT_START
     DIPS_HELPER (0x8000, "Z                    Y", KEYCODE_Z)
	 DIPS_HELPER (0x4000, "X", KEYCODE_X)
	 DIPS_HELPER (0x2000, "C", KEYCODE_C)
	 DIPS_HELPER (0x1000, "V", KEYCODE_V)
	 DIPS_HELPER (0x0800, "B", KEYCODE_B)
	 DIPS_HELPER (0x0400, "N", KEYCODE_N)
	 DIPS_HELPER (0x0200, "M", KEYCODE_M)
	 DIPS_HELPER (0x0100, ", <                    ;", KEYCODE_COMMA)
	 DIPS_HELPER (0x0080, ". >                    :", KEYCODE_STOP)
	 DIPS_HELPER (0x0040, "/ ?                  - _", KEYCODE_SLASH)
	 DIPS_HELPER (0x0020, "Right-Shift", KEYCODE_RSHIFT)
	 DIPS_HELPER (0x0010, "(Right-Shift Cursor-Down)CRSR-UP",
				  KEYCODE_8_PAD)
	 DIPS_HELPER (0x0008, "Space", KEYCODE_SPACE)
	 DIPS_HELPER (0x0004, "(Right-Shift Cursor-Right)CRSR-LEFT",
				  KEYCODE_4_PAD)
	 DIPS_HELPER (0x0002, "CRSR-DOWN", KEYCODE_2_PAD)
	 DIPS_HELPER (0x0001, "CRSR-RIGHT", KEYCODE_6_PAD)
     PORT_START
     DIPS_HELPER (0x8000, "STOP RUN", KEYCODE_ESC)
	 DIPS_HELPER (0x4000, "(C65)ESC", KEYCODE_F1)
	 DIPS_HELPER (0x2000, "(C65)ALT", KEYCODE_F2)
	 PORT_DIPNAME (0x1000, 0, "(C65)DIN ASC(switch)") PORT_CODE(KEYCODE_F3)
	PORT_DIPSETTING(  0, "ASC" )
	PORT_DIPSETTING(0x1000, "DIN" )
	DIPS_HELPER (0x0800, "(C65)NO SCRL", KEYCODE_F4)
	DIPS_HELPER (0x0400, "f1 f2", KEYCODE_F5)
	DIPS_HELPER (0x0200, "f3 f4", KEYCODE_F6)
	DIPS_HELPER (0x0100, "f5 f6", KEYCODE_F7)
	DIPS_HELPER (0x0080, "f7 f8", KEYCODE_F8)
	DIPS_HELPER (0x0040, "(C65)f9 f10", KEYCODE_F9)
	DIPS_HELPER (0x0020, "(C65)f11 f12", KEYCODE_F10)
	DIPS_HELPER (0x0010, "(C65)f13 f14", KEYCODE_F11)
	DIPS_HELPER (0x0008, "(C65)HELP", KEYCODE_F12)
INPUT_PORTS_END

static PALETTE_INIT( c65 )
{
	palette_set_colors(machine, 0, vic3_palette, sizeof(vic3_palette) / 3);
}

#if 0
	/* caff */
	/* dma routine alpha 1 (0x400000 reversed copy)*/
	ROM_LOAD ("910111.bin", 0x20000, 0x20000, CRC(c5d8d32e) SHA1(71c05f098eff29d306b0170e2c1cdeadb1a5f206))
	/* b96b */
	/* dma routine alpha 2 */
	ROM_LOAD ("910523.bin", 0x20000, 0x20000, CRC(e8235dd4) SHA1(e453a8e7e5b95de65a70952e9d48012191e1b3e7))
	/* 888c */
	/* dma routine alpha 2 */
	ROM_LOAD ("910626.bin", 0x20000, 0x20000, CRC(12527742) SHA1(07c185b3bc58410183422f7ac13a37ddd330881b))
	/* c9cd */
	/* dma routine alpha 2 */
	ROM_LOAD ("910828.bin", 0x20000, 0x20000, CRC(3ee40b06) SHA1(b63d970727a2b8da72a0a8e234f3c30a20cbcb26))
	/* 4bcf loading demo disk??? */
	/* basic program stored at 0x4000 ? */
	/* dma routine alpha 2 */
	ROM_LOAD ("911001.bin", 0x20000, 0x20000, CRC(0888b50f) SHA1(129b9a2611edaebaa028ac3e3f444927c8b1fc5d))
	/* german e96a */
	/* dma routine alpha 1 */
	ROM_LOAD ("910429.bin", 0x20000, 0x20000, CRC(b025805c) SHA1(c3b05665684f74adbe33052a2d10170a1063ee7d))
#endif

ROM_START (c65)
	ROM_REGION (0x400000, REGION_CPU1, 0)
	ROM_LOAD ("911001.bin", 0x20000, 0x20000, CRC(0888b50f) SHA1(129b9a2611edaebaa028ac3e3f444927c8b1fc5d))
ROM_END

ROM_START (c65e)
	ROM_REGION (0x400000, REGION_CPU1, 0)
	ROM_LOAD ("910828.bin", 0x20000, 0x20000, CRC(3ee40b06) SHA1(b63d970727a2b8da72a0a8e234f3c30a20cbcb26))
ROM_END

ROM_START (c65d)
	ROM_REGION (0x400000, REGION_CPU1, 0)
	ROM_LOAD ("910626.bin", 0x20000, 0x20000, CRC(12527742) SHA1(07c185b3bc58410183422f7ac13a37ddd330881b))
ROM_END

ROM_START (c65c)
	ROM_REGION (0x400000, REGION_CPU1, 0)
	ROM_LOAD ("910523.bin", 0x20000, 0x20000, CRC(e8235dd4) SHA1(e453a8e7e5b95de65a70952e9d48012191e1b3e7))
ROM_END

ROM_START (c65ger)
	ROM_REGION (0x400000, REGION_CPU1, 0)
	ROM_LOAD ("910429.bin", 0x20000, 0x20000, CRC(b025805c) SHA1(c3b05665684f74adbe33052a2d10170a1063ee7d))
ROM_END

ROM_START (c65a)
	ROM_REGION (0x400000, REGION_CPU1, 0)
	ROM_LOAD ("910111.bin", 0x20000, 0x20000, CRC(c5d8d32e) SHA1(71c05f098eff29d306b0170e2c1cdeadb1a5f206))
ROM_END


static SID6581_interface c65_sound_interface =
{
	c64_paddle_read
};

static MACHINE_DRIVER_START( c65 )
	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M4510, 3500000)  /* or VIC6567_CLOCK, */
	MDRV_CPU_PROGRAM_MAP(c65_mem, 0)
	MDRV_CPU_VBLANK_INT(c64_frame_interrupt, 1)
	MDRV_CPU_PERIODIC_INT(vic3_raster_irq, TIME_IN_HZ(VIC2_HRETRACERATE))
	MDRV_SCREEN_REFRESH_RATE(VIC6567_VRETRACERATE)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(0)

	MDRV_MACHINE_START( c65 )

	/* video hardware */
	MDRV_IMPORT_FROM( vh_vic2 )
	MDRV_SCREEN_SIZE(656, 416)
	MDRV_SCREEN_VISIBLE_AREA(0, 656 - 1, 0, 416 - 1)
	MDRV_PALETTE_LENGTH(sizeof(vic3_palette) / sizeof(vic3_palette[0]) / 3)
	MDRV_PALETTE_INIT( c65 )

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")
	MDRV_SOUND_ADD_TAG("sid_r", SID8580, 985248)
	MDRV_SOUND_CONFIG(c65_sound_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)
	MDRV_SOUND_ADD_TAG("sid_l", SID8580, 985248)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( c65pal )
	MDRV_IMPORT_FROM( c65 )
	MDRV_SCREEN_REFRESH_RATE(VIC6569_VRETRACERATE)

	MDRV_SOUND_REPLACE("sid_r", SID8580, 1022727)
	MDRV_SOUND_CONFIG(c65_sound_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)
	MDRV_SOUND_ADD_TAG("sid_l", SID8580, 1022727)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
MACHINE_DRIVER_END

static DRIVER_INIT( c65 )			{ c65_driver_init(); }
static DRIVER_INIT( c65_alpha1 )	{ c65_driver_init(); }
static DRIVER_INIT( c65pal )		{ c65pal_driver_init(); }

static void c65_quickload_getinfo(const device_class *devclass, UINT32 state, union devinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_FILE_EXTENSIONS:				strcpy(info->s = device_temp_str(), "p00,prg"); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_PTR_QUICKLOAD_LOAD:				info->f = (genf *) quickload_load_cbm_c65; break;

		/* --- the following bits of info are returned as doubles --- */
		case DEVINFO_FLOAT_QUICKLOAD_DELAY:				info->d = CBM_QUICKLOAD_DELAY; break;

		default:										quickload_device_getinfo(devclass, state, info); break;
	}
}

SYSTEM_CONFIG_START(c65)
	CONFIG_DEVICE(cbmfloppy_device_getinfo)
	CONFIG_DEVICE(c65_quickload_getinfo)
	CONFIG_RAM_DEFAULT(128 * 1024)
	CONFIG_RAM((128 + 512) * 1024)
	CONFIG_RAM((128 + 4096) * 1024)
SYSTEM_CONFIG_END

/*		YEAR	NAME	PARENT	COMPAT	MACHINE INPUT	INIT		CONFIG  COMPANY 							FULLNAME */
COMP ( 1991,	c65,	0,		0,		c65,	c65,	c65,		c65,	"Commodore Business Machines Co.",  "C65 / C64DX (Prototype, NTSC, 911001)",        GAME_NOT_WORKING)
COMP ( 1991,	c65e,	c65,	0,		c65,	c65,	c65,		c65,	"Commodore Business Machines Co.",  "C65 / C64DX (Prototype, NTSC, 910828)",        GAME_NOT_WORKING)
COMP ( 1991,	c65d,	c65,	0,		c65,	c65,	c65,		c65,	"Commodore Business Machines Co.",  "C65 / C64DX (Prototype, NTSC, 910626)",        GAME_NOT_WORKING)
COMP ( 1991,	c65c,	c65,	0,		c65,	c65,	c65,		c65,	"Commodore Business Machines Co.",  "C65 / C64DX (Prototype, NTSC, 910523)",        GAME_NOT_WORKING)
COMP ( 1991,	c65ger, c65,	0,		c65pal, c65ger, c65pal, 	c65,	"Commodore Business Machines Co.",  "C65 / C64DX (Prototype, German PAL, 910429)",  GAME_NOT_WORKING)
COMP ( 1991,	c65a,	c65,	0,		c65,	c65,	c65_alpha1, c65,	"Commodore Business Machines Co.",  "C65 / C64DX (Prototype, NTSC, 910111)",        GAME_NOT_WORKING)
