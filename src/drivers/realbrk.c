/***************************************************************************

					  -= Billiard Academy Real Break =-

					driver by	Luca Elia (l.elia@tin.it)


CPU    :	TMP68301

Sound  :	YMZ280b + YM2413

Chips  :	4L10F2467 (QFP160)
			4L10F2468 (QFP160)
			ACTEL A1010A
			PST532 (system reset and battery backup switching)


---------------------------------------------------------------------------
Year + Game							Board
---------------------------------------------------------------------------
98	Billiard Academy Real Break		NM523-1-9805
---------------------------------------------------------------------------

Notes:

- To reach the (hidden?) test modes put a breakpoint at 9f80, go in service
  mode and select "results" from the menu - this hits the breakpoint - then
  set ($3,a2) to non zero.

To Do:

- Priorities (e.g during the intro, there are two black bands in the backround
  that should obscure sprites).
- Sometimes sprites are shrinked to end up overlapping the background image
  in the tilemaps, but they are a few pixels off



Pachinko Gindama Shoubu DX
Nakanihon Inc. / Dynax, 1998

PCB Layout
----------

NM522-1-9803
|----------------------------------------------------------------------|
||-----|              |--------|                                       |
||68301| 52202B       |ACTEL   | M548262   YMZ280B          52204      |
||     | 52201B       |TPC1010 | M548262   32.768kHz 52203  52205      |
||-----|              |(PLCC68)|           M6242B   3V_BATT            |
|                     |--------||---------|  PST532A      |---------|  |
|            TC55257 33.8688MHz |NIKANIHON|               |NIKANIHON|  |
|            TC55257            |4L10F2468|    32.000MHz  |4L10F2467|  |
|2            TC55257           |         |               |         |  |
|8            TC55257           |(QFP160) |               |(QFP160) |  |
|W                              |---------| PAL           |---------|  |
|A                           LC3664     52206     52208     52210      |
|Y      VOL1                 LC3664          52207     52209     52211 |
|       VOL2                     LC3664                                |
| TA7252      3404               LC3664                                |
|     3404                                                             |
|  YAC516  YM2413        TC55257                                       |
|            3.579545MHz TC55257                                       |
| DSW2(10) DSW1(10)                                                    |
| DSW3(10) DSW4(10)             *    *    *    *    *    *    *    *   |
|                        TC55257                                       |
| CN3                    TC55257                                       |
| CN4                                                                  |
|----------------------------------------------------------------------|

Notes:
              *: unpopulated 32 pin sockets
         PST532: IC for system reset and battery backup switching
        DIPSW's: Each have 10 switches.
      VOL1/VOL2: Separate volume levels for sound and voice
        CN3/CN4: Connectors for player 3 & 4 controls
        TC55257: 32K x8 SRAM
         LC3664: 8K x8 SRAM
        M548262: 256K x8 fastpage VRAM
          28WAY: Edge connector is JAMMA backwards (GND is pin 28,27; +5V is pins 26,25 etc)
                 and with RBG locations moved (i.e. not standard JAMMA)

  Vertical Sync: 60Hz
    Horiz. Sync: 15.81kHz
    68301 clock: 16.000MHz (32 / 2)
  YMZ280B clock: 16.9344MHz (33.8688 / 2)
   YM2413 clock: 3.579545MHz
OKI M6242 clock: 32.768kHz


Pachinko Gindama Shoubu
Nakanihon Inc. / Dynax, 199x

PCB Layout
----------

NM5050905-1
|----------------------------------------------------------------------|
| TA8201 75074 YM2413          |-----|     |------------------------|  |
|   PST532A  3V_BATT 32.768kHz |68301|  CN2|------------------------|  |
|        75074  M6242B         |     |     TC55257   YMZ280B           |
|  YAC513      3.579545MHz     |-----|     TC55257             TC55257 |
|                              |--------|                              |
|                              |ACTEL   |                      TC55257 |
|                              |A1010B  |                              |
|                              |(PLCC68)|                      TC55257 |
|                              |--------|                              |
|J         TC55257                                             TC55257 |
|A                                                                     |
|M         TC55257                                                     |
|M                                  M548262                            |
|A                                  M548262         32.000MHz          |
|                                               TC55257                |
|                                  |---------|             |---------| |
|    DSW1(10)                      |NIKANIHON|  TC55257    |NIKANIHON| |
|                                  |4L10F2468|             |4L10F2467| |
|    DSW2(10)     TC55257          |         |             |         | |
|                                  |(QFP160) |             |(QFP160) | |
|                 TC55257          |---------|             |---------| |
|   CN6                                    |------------------------|  |
|   CN7              33.8688MHz         CN3|------------------------|  |
|----------------------------------------------------------------------|
Notes:
              *: unpopulated 32 pin sockets
         PST532: IC for system reset and battery backup switching
        DIPSW's: Each have 10 switches.
        CN2/CN3: ROM daughterboard connectors
        CN6/CN7: Connectors for player 3 & 4 controls
        TC55257: 32K x8 SRAM
        M548262: 256K x8 fastpage VRAM

  Vertical Sync: 60Hz
    Horiz. Sync: 15.81kHz
    68301 clock: 16.000MHz (32 / 2)
  YMZ280B clock: 16.9344MHz (33.8688 / 2)
   YM2413 clock: 3.579545MHz
OKI M6242 clock: 32.768kHz


ROM Daughterboard
-----------------

NS5050905-2
|----------------------------------------|
|            |------------------------|  |
|         CN1|------------------------|  |
|                                        |
|  50505.1K     50507.1F    50510.1C     |
|     50506.1H      50508.1E    50511.1B |
|                                        |
|              PAL   50509.2D    50512.2A|
|                                        |
|              PAL                       |
|                                        |
|                                        |
|50503.3L          *    *    *    *      |
|      50504.3K                          |
|                                        |
|                                        |
|                                        |
|                                        |
|50501.4L          *    *    *    *      |
|      50502.4K                          |
|                                        |
|                                        |
|            |------------------------|  |
|         CN2|------------------------|  |
|----------------------------------------|
Notes:
      *: Unpopulated 32pin sockets



***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "machine/tmp68301.h"
#include "realbrk.h"

static data16_t *realbrk_dsw_select;

/* Read 4 ten bit dip switches */
static READ16_HANDLER( realbrk_dsw_r )
{
	data16_t sel = ~realbrk_dsw_select[0];
	if (sel & 0x01)	return	(readinputport(2) & 0x00ff) << 8;		// DSW1 low bits
	if (sel & 0x02)	return	(readinputport(3) & 0x00ff) << 8;		// DSW2 low bits
	if (sel & 0x04)	return	(readinputport(4) & 0x00ff) << 8;		// DSW3 low bits
	if (sel & 0x08)	return	(readinputport(5) & 0x00ff) << 8;		// DSW4 low bits

	if (sel & 0x10)	return	((readinputport(2) & 0x0300) << 0) |	// DSWs high 2 bits
							((readinputport(3) & 0x0300) << 2) |
							((readinputport(4) & 0x0300) << 4) |
							((readinputport(5) & 0x0300) << 6) ;

	logerror("CPU #0 PC %06X: read with unknown dsw_select = %02x\n",activecpu_get_pc(),realbrk_dsw_select[0]);
	return 0xffff;
}

/***************************************************************************

								Memory Maps

***************************************************************************/

static ADDRESS_MAP_START( realbrk_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(MRA16_ROM					)	// ROM
	AM_RANGE(0x200000, 0x203fff) AM_READ(MRA16_RAM					)	// Sprites
	AM_RANGE(0x400000, 0x40ffff) AM_READ(MRA16_RAM					)	// Palette
	AM_RANGE(0x600000, 0x601fff) AM_READ(MRA16_RAM					)	// Background	(0)
	AM_RANGE(0x602000, 0x603fff) AM_READ(MRA16_RAM					)	// Background	(1)
	AM_RANGE(0x604000, 0x604fff) AM_READ(MRA16_RAM					)	// Text			(2)
	AM_RANGE(0x606000, 0x60600f) AM_READ(MRA16_RAM					)	// Scroll + Video Regs
	AM_RANGE(0x605000, 0x61ffff) AM_READ(MRA16_RAM					)	//
	AM_RANGE(0x800002, 0x800003) AM_READ(YMZ280B_status_0_msb_r	)	// YMZ280
	AM_RANGE(0xc00000, 0xc00001) AM_READ(input_port_0_word_r		)	// P1 & P2 (Inputs)
	AM_RANGE(0xc00002, 0xc00003) AM_READ(input_port_1_word_r		)	// Coins
	AM_RANGE(0xc00004, 0xc00005) AM_READ(realbrk_dsw_r				)	// 4 x DSW (10 bits each)
	AM_RANGE(0xfe0000, 0xfeffff) AM_READ(MRA16_RAM					)	// RAM
	AM_RANGE(0xff0000, 0xfffbff) AM_READ(MRA16_RAM					)	// RAM
	AM_RANGE(0xfffc00, 0xffffff) AM_READ(MRA16_RAM					)	// TMP68301 Registers
ADDRESS_MAP_END

static ADDRESS_MAP_START( realbrk_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(MWA16_ROM							)	// ROM
	AM_RANGE(0x200000, 0x203fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16			)	// Sprites
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16	)	// Palette
	AM_RANGE(0x600000, 0x601fff) AM_WRITE(realbrk_vram_0_w) AM_BASE(&realbrk_vram_0	)	// Background	(0)
	AM_RANGE(0x602000, 0x603fff) AM_WRITE(realbrk_vram_1_w) AM_BASE(&realbrk_vram_1	)	// Background	(1)
	AM_RANGE(0x604000, 0x604fff) AM_WRITE(realbrk_vram_2_w) AM_BASE(&realbrk_vram_2	)	// Text			(2)
	AM_RANGE(0x606000, 0x60600f) AM_WRITE(realbrk_vregs_w) AM_BASE(&realbrk_vregs 	)	// Scroll + Video Regs
	AM_RANGE(0x605000, 0x61ffff) AM_WRITE(MWA16_RAM							)	//
	AM_RANGE(0x800000, 0x800001) AM_WRITE(YMZ280B_register_0_msb_w			)	// YMZ280
	AM_RANGE(0x800002, 0x800003) AM_WRITE(YMZ280B_data_0_msb_w				)	//
	AM_RANGE(0x800008, 0x800009) AM_WRITE(YM2413_register_port_0_lsb_w		)	// YM2413
	AM_RANGE(0x80000a, 0x80000b) AM_WRITE(YM2413_data_port_0_lsb_w			)	//
	AM_RANGE(0xc00004, 0xc00005) AM_WRITE(MWA16_RAM) AM_BASE(&realbrk_dsw_select	)	// DSW select
	AM_RANGE(0xfe0000, 0xfeffff) AM_WRITE(MWA16_RAM							)	// RAM
	AM_RANGE(0xff0000, 0xfffbff) AM_WRITE(MWA16_RAM							)	// RAM
	AM_RANGE(0xfffd0a, 0xfffd0b) AM_WRITE(realbrk_flipscreen_w				)	// Hack! Parallel port data register
	AM_RANGE(0xfffc00, 0xffffff) AM_WRITE(tmp68301_regs_w) AM_BASE(&tmp68301_regs	)	// TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************

								Input Ports

***************************************************************************/

/***************************************************************************
						Billiard Academy Real Break
***************************************************************************/

INPUT_PORTS_START( realbrk )
	PORT_START	// IN0 - $c00000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNUSED )	// BUTTON3 in test mode
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x4000, IP_ACTIVE_LOW, IPT_UNUSED )	// BUTTON3 in test mode
	PORT_BIT(  0x8000, IP_ACTIVE_LOW, IPT_START1 ) PORT_PLAYER(1)

	PORT_START	// IN1 - $c00002.w
	PORT_BIT(  0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0100, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT(  0x0200, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT(  0x0400, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT(  0x1000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x2000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )	// the vblank routine wants these 2 bits high
	PORT_BIT(  0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START	// IN2 - $c00004.w (DSW1)
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0200, IP_ACTIVE_LOW )

	PORT_START	// IN3 - $c00004.w (DSW2)
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPSETTING(      0x0001, "5" )
	PORT_DIPSETTING(      0x0003, "6" )
	PORT_DIPSETTING(      0x0002, "7" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0004, DEF_STR( Harder )  )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0030, 0x0030, "Time" )
	PORT_DIPSETTING(      0x0000, "110" )
	PORT_DIPSETTING(      0x0030, "120" )
	PORT_DIPSETTING(      0x0020, "130" )
	PORT_DIPSETTING(      0x0010, "150" )
	PORT_DIPNAME( 0x0040, 0x0040, "? Show time in easy mode ?" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0080, "? Coins ?" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0080, "2" )

	PORT_START	// IN4 - $c00004.w (DSW3) - Unused
	PORT_BIT(  0xffff, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START	// IN5 - $c00004.w (DSW4) - Unused
	PORT_BIT(  0xffff, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************

							Graphics Layouts

***************************************************************************/

static struct GfxLayout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{	STEP4(0,1)		},
	{	STEP4(3*4,-4),STEP4(7*4,-4)		},
	{	STEP8(0,8*4)	},
	8*8*4
};

static struct GfxLayout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{	STEP4(0,1)		},
	{	1*4,0*4,3*4,2*4,5*4,4*4,7*4,6*4,9*4,8*4,11*4,10*4,13*4,12*4,15*4,14*4 },
	{	STEP16(0,16*4)	},
	16*16*4
};

static struct GfxLayout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{	STEP8(0,1)		},
	{	STEP16(0,8)		},
	{	STEP16(0,16*8)	},
	16*16*8
};

static struct GfxDecodeInfo realbrk_gfxdecodeinfo[] =
{
	{ REGION_GFX1, 0, &layout_16x16x8,		0, 0x80		},	// [0] Backgrounds
	{ REGION_GFX2, 0, &layout_8x8x4,		0, 0x800	},	// [1] Text
	{ REGION_GFX3, 0, &layout_16x16x8,		0, 0x80		},	// [2] Sprites (256 colors)
	{ REGION_GFX4, 0, &layout_16x16x4,		0, 0x800	},	// [3] Sprites (16 colors)
	{ -1 }
};


/***************************************************************************

								Machine Drivers

***************************************************************************/

/***************************************************************************
						Billiard Academy Real Break
***************************************************************************/

static struct YMZ280Binterface realbrk_ymz280b_intf =
{
	1,
	{ 33868800 / 2 },
	{ REGION_SOUND1 },
	{ YM3012_VOL(50,MIXER_PAN_LEFT,50,MIXER_PAN_RIGHT) },
	{ 0 }
};

static struct YM2413interface realbrk_ym2413_intf =
{
	1,	/* 1 chip */
	3579000,	/* 3.579 MHz */
	{ YM2413_VOL(50,MIXER_PAN_CENTER,50,MIXER_PAN_CENTER) }
};

static INTERRUPT_GEN( realbrk_interrupt )
{
	switch ( cpu_getiloops() )
	{
		case 0:
			/* VBlank is connected to INT1 (external interrupts pin 1) */
			tmp68301_external_interrupt_1();
			break;
	}
}

static MACHINE_DRIVER_START( realbrk )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main",M68000,32000000 / 2)			/* !! TMP68301 !! */
	MDRV_CPU_PROGRAM_MAP(realbrk_readmem,realbrk_writemem)
	MDRV_CPU_VBLANK_INT(realbrk_interrupt,1)

	MDRV_FRAMES_PER_SECOND(60)
	MDRV_VBLANK_DURATION(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_MACHINE_INIT( tmp68301 )

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_SIZE(0x140, 0xe0)
	MDRV_VISIBLE_AREA(0, 0x140-1, 0, 0xe0-1)
	MDRV_GFXDECODE(realbrk_gfxdecodeinfo)
	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_VIDEO_START(realbrk)
	MDRV_VIDEO_UPDATE(realbrk)

	/* sound hardware */
	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD(YMZ280B,	realbrk_ymz280b_intf)
	MDRV_SOUND_ADD(YM2413,	realbrk_ym2413_intf)
MACHINE_DRIVER_END

/***************************************************************************

								ROMs Loading

***************************************************************************/

/***************************************************************************

						Billiard Academy Real Break

Dynax, 199?

PCB Layout
----------


NM523-1-9805
------------------------------------------------------------------------
|                                                                      |
|  68301-16    52302    ACTEL    M548262   YMZ280B     *    52304      |
|              52301    A1010A   M548262             52303  52305      |
|                             33.8688MHz                               |
|                      TC55257               PST532A   %               |
|                      TC55257   4L10F2468                4L10F2467    |
|                      TC55257   (QFP160)      32.000MHz  (QFP160)     |
| J                    TC55257                                         |
| A                                     PAL                            |
| M                                                                    |
| M                              LH5160 52306     52308     52310      |
| A     VOL1                     LH5160      52307     52309     52311 |
|       VOL2                     LH5160                                |
|                                LH5160                                |
|                                                                      |
|          YM2413                TC55257                               |
|                    3.579MHz    TC55257  *  52312  *    *    *    *   |
|                                                                      |
|   YAC516                                                             |
|                             TC55257  TC55257                         |
|   DSW2(10) DSW1(10)         TC55257  TC55257                         |
|                                                                      |
------------------------------------------------------------------------

Notes:
            *: unpopulated 42 pin sockets.
            %: unpopulated position for coin battery.
       PST532: IC for system reset and battery backup switching
      DIPSW's: Each have 10 switches.
    VOL1/VOL2: Separate volume levels for sound and voice
Vertical Sync: 60Hz
  Horiz. Sync: 15.81kHz


***************************************************************************/

ROM_START( realbrk )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "52302.1r", 0x000000, 0x080000, CRC(ab0379b0) SHA1(67af6670f2b37a7d4d6e03508f291f8ffe64d4cb) )
	ROM_LOAD16_BYTE( "52301.2r", 0x000001, 0x080000, CRC(9cc1596e) SHA1(a598f18eaac1ed6943069e9500b07b77e263f0d0) )

	ROM_REGION( 0x800000, REGION_GFX1, ROMREGION_DISPOSE )	/* Backgrounds */
	ROM_LOAD32_WORD( "52310.9b", 0x0000000, 0x400000, CRC(07dfd9f5) SHA1(8722a98adc33f56df1e3b194ce923bc987e15cbe) )
	ROM_LOAD32_WORD( "52311.9a", 0x0000002, 0x400000, CRC(136a93a4) SHA1(b4bd46ba6c2b367aaf362f67d8be4757f1160864) )

	ROM_REGION( 0x40000, REGION_GFX2, ROMREGION_DISPOSE )	/* Text Layer */
	ROM_LOAD16_BYTE( "52305.1a", 0x000000, 0x020000, CRC(56546fb4) SHA1(5e4dc1665ca96bf24b89d92c24f5ff8420cb465e) )	// 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "52304.1b", 0x000001, 0x020000, CRC(b22b0aac) SHA1(8c62e19071a4031d0dcad621cce0ba550702659b) )	// 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0xc00000, REGION_GFX3, ROMREGION_DISPOSE )	/* Sprites (256 colors) */
	ROM_LOAD32_WORD( "52306.9f", 0x0000000, 0x400000, CRC(5ff0f666) SHA1(e3f1d9dc84fbef73af37cefd90bdf87a35f59e0e) )
	ROM_LOAD32_WORD( "52308.9d", 0x0000002, 0x400000, CRC(20817051) SHA1(4c9a443b5d6353ce67d5b1fe716f5ac20d194ef0) )
	ROM_LOAD32_WORD( "52307.9e", 0x0800000, 0x200000, CRC(01555191) SHA1(7751e2e16345acc638d4dff997a5b52e9171fced) )
	ROM_LOAD32_WORD( "52309.9c", 0x0800002, 0x200000, CRC(ef4f4bd9) SHA1(3233f501002a2622ddda581167ae24b1a13ea79e) )

	ROM_REGION( 0x200000, REGION_GFX4, ROMREGION_DISPOSE )	/* Sprites (16 colors) */
	ROM_LOAD( "52312.14f", 0x000000, 0x200000, CRC(2203d7c5) SHA1(0403f02b8f2bfc6cf98ff598eb9c2e3facc7ac4c) )

	ROM_REGION( 0x400000, REGION_SOUND1, ROMREGION_SOUNDONLY )	/* Samples */
	ROM_LOAD( "52303.2e", 0x000000, 0x400000, CRC(d3005b1e) SHA1(3afd10cdbc3aa7605083a9fcf3c4b8276937c2c4) )
ROM_END

ROM_START( pkgnsh )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "50506.1h", 0x000001, 0x080000, CRC(06949a7d) SHA1(1276c28bc5cebeae749e0cded2da631353efbbb4) )
	ROM_LOAD16_BYTE( "50505.1k", 0x000000, 0x080000, CRC(26df869f) SHA1(d716e561441da6ae8ca61e17335aab44770157a6) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE )	/* Backgrounds */
	ROM_LOAD32_WORD( "50512.2a", 0x0000002, 0x200000, CRC(5adae7bb) SHA1(de7cf952155459f7aab1448620bf26a925ca0572) )
	ROM_LOAD32_WORD( "50509.2d", 0x0000000, 0x200000, CRC(ad937ab5) SHA1(ebe02c203358787c6b406fe3cbd3eca3b245456e) )

	ROM_REGION( 0x40000, REGION_GFX2, ROMREGION_DISPOSE )	/* Text Layer */
	ROM_LOAD16_BYTE( "50510.1c", 0x000001, 0x020000, CRC(e26f1ad6) SHA1(5713af5fb02885657889cb2df7f58a39987ace6a) )
	ROM_LOAD16_BYTE( "50511.1b", 0x000000, 0x020000, CRC(3da9af01) SHA1(21313fd5c8cf7ccb72c85422dbddfceedab2542f) )

	ROM_REGION( 0xc00000, REGION_GFX3, ROMREGION_DISPOSE )	/* Sprites (256 colors) */
	ROM_LOAD32_WORD( "50502.4k", 0x0000000, 0x400000, CRC(f7c04779) SHA1(fcbc2d166d405d0fe2a4ca67950fe6ec060b9fc1) ) // same as 52206.9f on dx
	ROM_LOAD32_WORD( "50504.3k", 0x0000002, 0x400000, CRC(8e872be5) SHA1(0568a70ca640624f665b8b92ca5e9239b13ed116) ) // same as 52208.9d on dx
	ROM_LOAD32_WORD( "50501.4l", 0x0800000, 0x200000, CRC(ca31e1ad) SHA1(7508547de5617f6091fc46f6eb1b45673419c483) )
	ROM_LOAD32_WORD( "50503.3l", 0x0800002, 0x200000, CRC(80b5e8d0) SHA1(27359affaa84c7cb4dfc019bbfeae0f384602faa) )

	ROM_REGION( 0x200000, REGION_GFX4, ROMREGION_DISPOSE )	/* Sprites (16 colors) Not Used */

	ROM_REGION( 0x100000, REGION_SOUND1, ROMREGION_SOUNDONLY )	/* Samples */
	ROM_LOAD( "50508.1e", 0x000000, 0x080000, CRC(95a1473a) SHA1(d382a9a603711747c2fe5bd5721de5af369ccc42) )
	ROM_LOAD( "50507.1f", 0x080000, 0x080000, CRC(34a003a1) SHA1(f3fa4de1f75e8fa18a8431a8c2ce495aa47989b9) )
ROM_END


ROM_START( pkgnshdx )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "52202b.1r", 0x000000, 0x080000, CRC(3c1a10de) SHA1(44a13adec64645aa01e216dfd527b59e7298c732) )
	ROM_LOAD16_BYTE( "52201b.2r", 0x000001, 0x080000, CRC(d63797ce) SHA1(d1b0b57b5426135e36772be296e94e04822e54ac) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_DISPOSE )	/* Backgrounds */
	ROM_LOAD32_WORD( "52210.9b", 0x0000000, 0x200000, CRC(6865b76a) SHA1(26215ec38b1fef279b3c3c1453116a0afe938b6b) )
	ROM_LOAD32_WORD( "52211.9a", 0x0000002, 0x200000, CRC(8e227328) SHA1(200f9e4419dac62b191e5e8c6c32b777a9c08e5e) )

	ROM_REGION( 0x40000, REGION_GFX2, ROMREGION_DISPOSE )	/* Text Layer */
	ROM_LOAD16_BYTE( "52205.1a", 0x000000, 0x020000, CRC(4b7d16c0) SHA1(5f6410121ec13bea2869d61db169dbe2536453ea) )
	ROM_LOAD16_BYTE( "52204.1b", 0x000001, 0x020000, CRC(47a39496) SHA1(3ac9499b70c63185fb65378c18d4ff30ba1d2f2b) )

	ROM_REGION( 0xc00000, REGION_GFX3, ROMREGION_DISPOSE )	/* Sprites (256 colors) */
	ROM_LOAD32_WORD( "52206.9f", 0x0000000, 0x400000, CRC(f7c04779) SHA1(fcbc2d166d405d0fe2a4ca67950fe6ec060b9fc1) )
	ROM_LOAD32_WORD( "52208.9d", 0x0000002, 0x400000, CRC(8e872be5) SHA1(0568a70ca640624f665b8b92ca5e9239b13ed116) )
	ROM_LOAD32_WORD( "52207.9e", 0x0800000, 0x200000, CRC(ae7a983f) SHA1(ba8ff28068e21dd24ea2e523a5b4023e86ea26cb) )
	ROM_LOAD32_WORD( "52209.9c", 0x0800002, 0x200000, CRC(83ac2ea9) SHA1(aa1c45b7a404eed51e950bea3edcd34814f09213) )

	ROM_REGION( 0x200000, REGION_GFX4, ROMREGION_DISPOSE )	/* Sprites (16 colors) Not Used */

	ROM_REGION( 0x100000, REGION_SOUND1, ROMREGION_SOUNDONLY )	/* Samples */
	ROM_LOAD( "52203.2e", 0x000000, 0x100000, CRC(342a193d) SHA1(1e75ec7ac48dcc8396a0fa6db14f2661c28f671c) )
ROM_END



GAMEX( 1998, realbrk,  0, realbrk, realbrk, 0, ROT0, "Nakanihon", "Billiard Academy Real Break (Japan)", GAME_IMPERFECT_GRAPHICS )
GAMEX( 1998, pkgnsh, 0, realbrk, realbrk, 0, ROT0, "Nakanihon / Dynax", "Pachinko Gindama Shoubu (Japan)", GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING )
GAMEX( 1998, pkgnshdx, 0, realbrk, realbrk, 0, ROT0, "Nakanihon / Dynax", "Pachinko Gindama Shoubu DX (Japan)", GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING )

