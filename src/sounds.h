// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2023 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  sounds.h
/// \brief Sound and music info

#ifndef __SOUNDS__
#define __SOUNDS__

// Customisable sounds for Skins
typedef enum
{
	SKSSPIN,
	SKSPUTPUT,
	SKSPUDPUD,
	SKSPLPAN1, // Ouchies
	SKSPLPAN2,
	SKSPLPAN3,
	SKSPLPAN4,
	SKSPLDET1, // Deaths
	SKSPLDET2,
	SKSPLDET3,
	SKSPLDET4,
	SKSPLVCT1, // Victories
	SKSPLVCT2,
	SKSPLVCT3,
	SKSPLVCT4,
	SKSTHOK,
	SKSSPNDSH,
	SKSZOOM,
	SKSSKID,
	SKSGASP,
	SKSJUMP,
	NUMSKINSOUNDS
} skinsound_t;

// free sfx for S_AddSoundFx()
#define NUMSFXFREESLOTS 9999 // Matches SOC Editor.
#define NUMSKINSFXSLOTS (MAXSKINS*NUMSKINSOUNDS)

//
// SoundFX struct.
//
typedef struct sfxinfo_struct sfxinfo_t;

struct sfxinfo_struct
{
	// up to 6-character name
	const char *name;

	// Sfx singularity (only one at a time)
	boolean singularity;

	// Sfx priority
	INT32 priority;

	// pitch if a link
	INT32 pitch;

	// volume if a link
	INT32 volume;

	// sound data
	void *data;

	// length of sound data
	size_t length;

	// sound that can be remapped for a skin, indexes skins[].skinsounds
	// 0 up to (NUMSKINSOUNDS-1), -1 = not skin specifc
	INT32 skinsound;

	// this is checked every second to see if sound
	// can be thrown out (if 0, then decrement, if -1,
	// then throw out, if > 0, then it is in use)
	INT32 usefulness;

	// lump number of sfx
	lumpnum_t lumpnum;

	// closed caption info/wiki table bait
	char caption[32];
};

// the complete set of sound effects
extern sfxinfo_t S_sfx[];

//
// Identifiers for all sfx in game.
//

// List of sounds (don't modify this comment!)
typedef enum
{
	sfx_None,

	// Skin Sounds
	sfx_altdi1,
	sfx_altdi2,
	sfx_altdi3,
	sfx_altdi4,
	sfx_altow1,
	sfx_altow2,
	sfx_altow3,
	sfx_altow4,
	sfx_victr1,
	sfx_victr2,
	sfx_victr3,
	sfx_victr4,
	sfx_gasp,
	sfx_jump,
	sfx_pudpud,
	sfx_putput,
	sfx_spin,
	sfx_spndsh,
	sfx_thok,
	sfx_zoom,
	sfx_skid,

	// Ambience/background objects/etc
	sfx_ambint,

	sfx_alarm,
	sfx_buzz1,
	sfx_buzz2,
	sfx_buzz3,
	sfx_buzz4,
	sfx_crumbl,
	sfx_fire,
	sfx_grind,
	sfx_laser,
	sfx_mswing,
	sfx_pstart,
	sfx_pstop,
	sfx_steam1,
	sfx_steam2,
	sfx_wbreak,
	sfx_ambmac,
	sfx_spsmsh,

	sfx_rainin,
	sfx_litng1,
	sfx_litng2,
	sfx_litng3,
	sfx_litng4,
	sfx_athun1,
	sfx_athun2,

	sfx_amwtr1,
	sfx_amwtr2,
	sfx_amwtr3,
	sfx_amwtr4,
	sfx_amwtr5,
	sfx_amwtr6,
	sfx_amwtr7,
	sfx_amwtr8,
	sfx_bubbl1,
	sfx_bubbl2,
	sfx_bubbl3,
	sfx_bubbl4,
	sfx_bubbl5,
	sfx_floush,
	sfx_splash,
	sfx_splish,
	sfx_wdrip1,
	sfx_wdrip2,
	sfx_wdrip3,
	sfx_wdrip4,
	sfx_wdrip5,
	sfx_wdrip6,
	sfx_wdrip7,
	sfx_wdrip8,
	sfx_wslap,

	sfx_doora1,
	sfx_doorb1,
	sfx_doorc1,
	sfx_doorc2,
	sfx_doord1,
	sfx_doord2,
	sfx_eleva1,
	sfx_eleva2,
	sfx_eleva3,
	sfx_elevb1,
	sfx_elevb2,
	sfx_elevb3,

	sfx_ambin2,
	sfx_lavbub,
	sfx_rocks1,
	sfx_rocks2,
	sfx_rocks3,
	sfx_rocks4,
	sfx_rumbam,
	sfx_rumble,

	// Game objects, etc
	sfx_appear,
	sfx_bkpoof,
	sfx_bnce1,
	sfx_bnce2,
	sfx_cannon,
	sfx_cgot,
	sfx_cybdth,
	sfx_deton,
	sfx_ding, // old sfx_appear sound
	sfx_dmpain,
	sfx_drown,
	sfx_fizzle,
	sfx_gbeep,
	sfx_wepfir,
	sfx_ghit,
	sfx_gloop,
	sfx_gspray,
	sfx_gravch,
	sfx_itemup,
	sfx_jet,
	sfx_jshard,
	sfx_lose,
	sfx_lvpass,
	sfx_mindig,
	sfx_mixup,
	sfx_monton,
	sfx_pogo,
	sfx_pop,
	sfx_rail1,
	sfx_rail2,
	sfx_rlaunc,
	sfx_shield,
	sfx_wirlsg,
	sfx_forcsg,
	sfx_frcssg,
	sfx_elemsg,
	sfx_armasg,
	sfx_attrsg,
	sfx_shldls,
	sfx_spdpad,
	sfx_spkdth,
	sfx_spring,
	sfx_statu1,
	sfx_statu2,
	sfx_strpst,
	sfx_supert,
	sfx_telept,
	sfx_tink,
	sfx_token,
	sfx_trfire,
	sfx_trpowr,
	sfx_turhit,
	sfx_wdjump,
	sfx_shrpsp,
	sfx_shrpgo,
	sfx_mswarp,
	sfx_mspogo,
	sfx_boingf,
	sfx_corkp,
	sfx_corkh,
	sfx_alart,
	sfx_vwre,
	sfx_bowl,
	sfx_chuchu,
	sfx_bsnipe,
	sfx_sprong,
	sfx_lvfal1,
	sfx_pscree,
	sfx_iceb,
	sfx_shattr,
	sfx_antiri,

	// Menu, interface
	sfx_chchng,
	sfx_dwnind,
	sfx_emfind,
	sfx_flgcap,
	sfx_menu1,
	sfx_oneup,
	sfx_ptally,
	sfx_radio,
	sfx_wepchg,
	sfx_wtrdng,
	sfx_zelda,
	sfx_adderr,
	sfx_notadd,
	sfx_addfil,

	// NiGHTS
	sfx_ideya,
	sfx_xideya, // Xmas
	sfx_nbmper,
	sfx_nxbump, // Xmas
	sfx_ncchip,
	sfx_ncitem,
	sfx_nxitem, // Xmas
	sfx_ngdone,
	sfx_nxdone, // Xmas
	sfx_drill1,
	sfx_drill2,
	sfx_ncspec,
	sfx_nghurt,
	sfx_ngskid,
	sfx_hoop1,
	sfx_hoop2,
	sfx_hoop3,
	sfx_hidden,
	sfx_prloop,
	sfx_timeup,
	sfx_ngjump,
	sfx_peww,

	// Halloween
	sfx_lntsit,
	sfx_lntdie,
	sfx_pumpkn,
	sfx_ghosty,

	// Mario
	sfx_koopfr,
	sfx_mario1,
	sfx_mario2,
	sfx_mario3,
	sfx_mario4,
	sfx_mario5,
	sfx_mario6,
	sfx_mario7,
	sfx_mario8,
	sfx_mario9,
	sfx_marioa,
	sfx_thwomp,

	// Black Eggman
	sfx_bebomb,
	sfx_bechrg,
	sfx_becrsh,
	sfx_bedeen,
	sfx_bedie1,
	sfx_bedie2,
	sfx_beeyow,
	sfx_befall,
	sfx_befire,
	sfx_beflap,
	sfx_begoop,
	sfx_begrnd,
	sfx_behurt,
	sfx_bejet1,
	sfx_belnch,
	sfx_beoutb,
	sfx_beragh,
	sfx_beshot,
	sfx_bestep,
	sfx_bestp2,
	sfx_bewar1,
	sfx_bewar2,
	sfx_bewar3,
	sfx_bewar4,
	sfx_bexpld,
	sfx_bgxpld,

	// Cy-Brak-Demon
	sfx_beelec, // Electric barrier ambience
	sfx_brakrl, // Rocket launcher
	sfx_brakrx, // Rocket explodes

	// S1 sounds
	sfx_s1a0,
	sfx_s1a1,
	sfx_s1a2,
	sfx_s1a3,
	sfx_s1a4,
	sfx_s1a5,
	sfx_s1a6,
	sfx_s1a7,
	sfx_s1a8,
	sfx_s1a9,
	sfx_s1aa,
	sfx_s1ab,
	sfx_s1ac,
	sfx_s1ad,
	sfx_s1ae,
	sfx_s1af,
	sfx_s1b0,
	sfx_s1b1,
	sfx_s1b2,
	sfx_s1b3,
	sfx_s1b4,
	sfx_s1b5,
	sfx_s1b6,
	sfx_s1b7,
	sfx_s1b8,
	sfx_s1b9,
	sfx_s1ba,
	sfx_s1bb,
	sfx_s1bc,
	sfx_s1bd,
	sfx_s1be,
	sfx_s1bf,
	sfx_s1c0,
	sfx_s1c1,
	sfx_s1c2,
	sfx_s1c3,
	sfx_s1c4,
	sfx_s1c5,
	sfx_s1c6,
	sfx_s1c7,
	sfx_s1c8,
	sfx_s1c9,
	sfx_s1ca,
	sfx_s1cb,
	sfx_s1cc,
	sfx_s1cd,
	sfx_s1ce,
	sfx_s1cf,

	// S2 sounds
	sfx_s220,
	sfx_s221,
	sfx_s222,
	sfx_s223,
	sfx_s224,
	sfx_s225,
	sfx_s226,
	sfx_s227,
	sfx_s228,
	sfx_s229,
	sfx_s22a,
	sfx_s22b,
	sfx_s22c,
	sfx_s22d,
	sfx_s22e,
	sfx_s22f,
	sfx_s230,
	sfx_s231,
	sfx_s232,
	sfx_s233,
	sfx_s234,
	sfx_s235,
	sfx_s236,
	sfx_s237,
	sfx_s238,
	sfx_s239,
	sfx_s23a,
	sfx_s23b,
	sfx_s23c,
	sfx_s23d,
	sfx_s23e,
	sfx_s23f,
	sfx_s240,
	sfx_s241,
	sfx_s242,
	sfx_s243,
	sfx_s244,
	sfx_s245,
	sfx_s246,
	sfx_s247,
	sfx_s248,
	sfx_s249,
	sfx_s24a,
	sfx_s24b,
	sfx_s24c,
	sfx_s24d,
	sfx_s24e,
	sfx_s24f,
	sfx_s250,
	sfx_s251,
	sfx_s252,
	sfx_s253,
	sfx_s254,
	sfx_s255,
	sfx_s256,
	sfx_s257,
	sfx_s258,
	sfx_s259,
	sfx_s25a,
	sfx_s25b,
	sfx_s25c,
	sfx_s25d,
	sfx_s25e,
	sfx_s25f,
	sfx_s260,

	// S3&K sounds
	sfx_s3k2b,
	sfx_s3k33,
	sfx_s3k34,
	sfx_s3k35,
	sfx_s3k36,
	sfx_s3k37,
	sfx_s3k38,
	sfx_s3k39,
	sfx_s3k3a,
	sfx_s3k3b,
	sfx_s3k3c,
	sfx_s3k3d,
	sfx_s3k3e,
	sfx_s3k3f,
	sfx_s3k40,
	sfx_s3k41,
	sfx_s3k42,
	sfx_s3k43,
	sfx_s3k44,
	sfx_s3k45,
	sfx_s3k46,
	sfx_s3k47,
	sfx_s3k48,
	sfx_s3k49,
	sfx_s3k4a,
	sfx_s3k4b,
	sfx_s3k4c,
	sfx_s3k4d,
	sfx_s3k4e,
	sfx_s3k4f,
	sfx_s3k50,
	sfx_s3k51,
	sfx_s3k52,
	sfx_s3k53,
	sfx_s3k54,
	sfx_s3k55,
	sfx_s3k56,
	sfx_s3k57,
	sfx_s3k58,
	sfx_s3k59,
	sfx_s3k5a,
	sfx_s3k5b,
	sfx_s3k5c,
	sfx_s3k5d,
	sfx_s3k5e,
	sfx_s3k5f,
	sfx_s3k60,
	sfx_s3k61,
	sfx_s3k62,
	sfx_s3k63,
	sfx_s3k64,
	sfx_s3k65,
	sfx_s3k66,
	sfx_s3k67,
	sfx_s3k68,
	sfx_s3k69,
	sfx_s3k6a,
	sfx_s3k6b,
	sfx_s3k6c,
	sfx_s3k6d,
	sfx_s3k6e,
	sfx_s3k6f,
	sfx_s3k70,
	sfx_s3k71,
	sfx_s3k72,
	sfx_s3k73,
	sfx_s3k74,
	sfx_s3k75,
	sfx_s3k76,
	sfx_s3k77,
	sfx_s3k78,
	sfx_s3k79,
	sfx_s3k7a,
	sfx_s3k7b,
	sfx_s3k7c,
	sfx_s3k7d,
	sfx_s3k7e,
	sfx_s3k7f,
	sfx_s3k80,
	sfx_s3k81,
	sfx_s3k82,
	sfx_s3k83,
	sfx_s3k84,
	sfx_s3k85,
	sfx_s3k86,
	sfx_s3k87,
	sfx_s3k88,
	sfx_s3k89,
	sfx_s3k8a,
	sfx_s3k8b,
	sfx_s3k8c,
	sfx_s3k8d,
	sfx_s3k8e,
	sfx_s3k8f,
	sfx_s3k90,
	sfx_s3k91,
	sfx_s3k92,
	sfx_s3k93,
	sfx_s3k94,
	sfx_s3k95,
	sfx_s3k96,
	sfx_s3k97,
	sfx_s3k98,
	sfx_s3k99,
	sfx_s3k9a,
	sfx_s3k9b,
	sfx_s3k9c,
	sfx_s3k9d,
	sfx_s3k9e,
	sfx_s3k9f,
	sfx_s3ka0,
	sfx_s3ka1,
	sfx_s3ka2,
	sfx_s3ka3,
	sfx_s3ka4,
	sfx_s3ka5,
	sfx_s3ka6,
	sfx_s3ka7,
	sfx_s3ka8,
	sfx_s3ka9,
	sfx_s3kaa,
	sfx_s3kab,
	sfx_s3kab1,
	sfx_s3kab2,
	sfx_s3kab3,
	sfx_s3kab4,
	sfx_s3kab5,
	sfx_s3kab6,
	sfx_s3kab7,
	sfx_s3kab8,
	sfx_s3kab9,
	sfx_s3kaba,
	sfx_s3kabb,
	sfx_s3kabc,
	sfx_s3kabd,
	sfx_s3kabe,
	sfx_s3kabf,
	sfx_s3kac,
	sfx_s3kad,
	sfx_s3kae,
	sfx_s3kaf,
	sfx_s3kb0,
	sfx_s3kb1,
	sfx_s3kb2,
	sfx_s3kb3,
	sfx_s3kb4,
	sfx_s3kb5,
	sfx_s3kb6,
	sfx_s3kb7,
	sfx_s3kb8,
	sfx_s3kb9,
	sfx_s3kba,
	sfx_s3kbb,
	sfx_s3kbcs,
	sfx_s3kbcl,
	sfx_s3kbds,
	sfx_s3kbdl,
	sfx_s3kbes,
	sfx_s3kbel,
	sfx_s3kbfs,
	sfx_s3kbfl,
	sfx_s3kc0s,
	sfx_s3kc0l,
	sfx_s3kc1s,
	sfx_s3kc1l,
	sfx_s3kc2s,
	sfx_s3kc2l,
	sfx_s3kc3s,
	sfx_s3kc3l,
	sfx_s3kc4s,
	sfx_s3kc4l,
	sfx_s3kc5s,
	sfx_s3kc5l,
	sfx_s3kc6s,
	sfx_s3kc6l,
	sfx_s3kc7s,
	sfx_s3kc7l,
	sfx_s3kc8s,
	sfx_s3kc8l,
	sfx_s3kc9s,
	sfx_s3kc9l,
	sfx_s3kcas,
	sfx_s3kcal,
	sfx_s3kcbs,
	sfx_s3kcbl,
	sfx_s3kccs,
	sfx_s3kccl,
	sfx_s3kcds,
	sfx_s3kcdl,
	sfx_s3kces,
	sfx_s3kcel,
	sfx_s3kcfs,
	sfx_s3kcfl,
	sfx_s3kd0s,
	sfx_s3kd0l,
	sfx_s3kd1s,
	sfx_s3kd1l,
	sfx_s3kd2s,
	sfx_s3kd2l,
	sfx_s3kd3s,
	sfx_s3kd3l,
	sfx_s3kd4s,
	sfx_s3kd4l,
	sfx_s3kd5s,
	sfx_s3kd5l,
	sfx_s3kd6s,
	sfx_s3kd6l,
	sfx_s3kd7s,
	sfx_s3kd7l,
	sfx_s3kd8s,
	sfx_s3kd8l,
	sfx_s3kd9s,
	sfx_s3kd9l,
	sfx_s3kdas,
	sfx_s3kdal,
	sfx_s3kdbs,
	sfx_s3kdbl,

	// 3DB sounds
	sfx_3db06,
	sfx_3db09,
	sfx_3db14,
	sfx_3db16,

	// SCD sounds
	sfx_cdfm00,
	sfx_cdfm01,
	sfx_cdfm02,
	sfx_cdfm03,
	sfx_cdfm04,
	sfx_cdfm05,
	sfx_cdfm06,
	sfx_cdfm07,
	sfx_cdfm08,
	sfx_cdfm09,
	sfx_cdfm10,
	sfx_cdfm11,
	sfx_cdfm12,
	sfx_cdfm13,
	sfx_cdfm14,
	sfx_cdfm15,
	sfx_cdfm16,
	sfx_cdfm17,
	sfx_cdfm18,
	sfx_cdfm19,
	sfx_cdfm20,
	sfx_cdfm21,
	sfx_cdfm22,
	sfx_cdfm23,
	sfx_cdfm24,
	sfx_cdfm25,
	sfx_cdfm26,
	sfx_cdfm27,
	sfx_cdfm28,
	sfx_cdfm29,
	sfx_cdfm30,
	sfx_cdfm31,
	sfx_cdfm32,
	sfx_cdfm33,
	sfx_cdfm34,
	sfx_cdfm35,
	sfx_cdfm36,
	sfx_cdfm37,
	sfx_cdfm38,
	sfx_cdfm39,
	sfx_cdfm40,
	sfx_cdfm41,
	sfx_cdfm42,
	sfx_cdfm43,
	sfx_cdfm44,
	sfx_cdfm45,
	sfx_cdfm46,
	sfx_cdfm47,
	sfx_cdfm48,
	sfx_cdfm49,
	sfx_cdfm50,
	sfx_cdfm51,
	sfx_cdfm52,
	sfx_cdfm53,
	sfx_cdfm54,
	sfx_cdfm55,
	sfx_cdfm56,
	sfx_cdfm57,
	sfx_cdfm58,
	sfx_cdfm59,
	sfx_cdfm60,
	sfx_cdfm61,
	sfx_cdfm62,
	sfx_cdfm63,
	sfx_cdfm64,
	sfx_cdfm65,
	sfx_cdfm66,
	sfx_cdfm67,
	sfx_cdfm68,
	sfx_cdfm69,
	sfx_cdfm70,
	sfx_cdfm71,
	sfx_cdfm72,
	sfx_cdfm73,
	sfx_cdfm74,
	sfx_cdfm75,
	sfx_cdfm76,
	sfx_cdfm77,
	sfx_cdfm78,
	sfx_cdfm79,
	sfx_cdpcm0,
	sfx_cdpcm1,
	sfx_cdpcm2,
	sfx_cdpcm3,
	sfx_cdpcm4,
	sfx_cdpcm5,
	sfx_cdpcm6,
	sfx_cdpcm7,
	sfx_cdpcm8,
	sfx_cdpcm9,

	// KC sounds
	sfx_kc2a,
	sfx_kc2b,
	sfx_kc2c,
	sfx_kc2d,
	sfx_kc2e,
	sfx_kc2f,
	sfx_kc30,
	sfx_kc31,
	sfx_kc32,
	sfx_kc33,
	sfx_kc34,
	sfx_kc35,
	sfx_kc36,
	sfx_kc37,
	sfx_kc38,
	sfx_kc39,
	sfx_kc3a,
	sfx_kc3b,
	sfx_kc3c,
	sfx_kc3d,
	sfx_kc3e,
	sfx_kc3f,
	sfx_kc40,
	sfx_kc41,
	sfx_kc42,
	sfx_kc43,
	sfx_kc44,
	sfx_kc45,
	sfx_kc46,
	sfx_kc47,
	sfx_kc48,
	sfx_kc49,
	sfx_kc4a,
	sfx_kc4b,
	sfx_kc4c,
	sfx_kc4d,
	sfx_kc4e,
	sfx_kc4f,
	sfx_kc50,
	sfx_kc51,
	sfx_kc52,
	sfx_kc53,
	sfx_kc54,
	sfx_kc55,
	sfx_kc56,
	sfx_kc57,
	sfx_kc58,
	sfx_kc59,
	sfx_kc5a,
	sfx_kc5b,
	sfx_kc5c,
	sfx_kc5d,
	sfx_kc5e,
	sfx_kc5f,
	sfx_kc60,
	sfx_kc61,
	sfx_kc62,
	sfx_kc63,
	sfx_kc64,
	sfx_kc65,
	sfx_kc66,
	sfx_kc67,
	sfx_kc68,
	sfx_kc69,
	sfx_kc6b,
	sfx_kc6c,
	sfx_kc6d,
	sfx_kc6e,

	// free slots for S_AddSoundFx() at run-time --------------------
	sfx_freeslot0,
	//
	// ... 60 free sounds here ...
	//
	sfx_lastfreeslot = sfx_freeslot0 + NUMSFXFREESLOTS-1,
	// end of freeslots ---------------------------------------------

	sfx_skinsoundslot0,
	sfx_lastskinsoundslot = sfx_skinsoundslot0 + NUMSKINSFXSLOTS-1,
	NUMSFX
} sfxenum_t;


void S_InitRuntimeSounds(void);
sfxenum_t S_AddSoundFx(const char *name, boolean singular, INT32 flags, boolean skinsound);
extern sfxenum_t sfxfree; // sound test and slotting
void S_RemoveSoundFx(sfxenum_t id);

#endif
