#ifndef _ASM_RB_BOARDS_H
#define _ASM_RB_BOARDS_H

#define MACH_GROUP_MT_RB500    1	/* Mikrotik RB500 */
#define MACH_GROUP_MT_RB100    2	/* Mikrotik RB100 */
#define MACH_GROUP_MT_CR       3	/* Mikrotik CR */
#define MACH_GROUP_MT_RB400    4
#define MACH_GROUP_MT_VM       6
#define MACH_GROUP_MT_RB700    7
#define MACH_GROUP_MT_MUSIC    9
#define MACH_GROUP_MT_MMIPS    11


#define  MACH_MT_RB500		0
#define  MACH_MT_RB500R5	1
#define  MACH_MT_RB100		2
#define  MACH_MT_RB150		3
#define  MACH_MT_RB133		4
#define  MACH_MT_RB133C		5
#define  MACH_MT_MR		6
#define  MACH_MT_RB192		7
#define  MACH_MT_CR1		8
#define  MACH_MT_RB411		10
#define  MACH_MT_RB433		11
#define  MACH_MT_RB433U		12	/* RB433 + USB */
#define  MACH_MT_RB450		13
#define  MACH_MT_RB493		15
#define  MACH_MT_RB450G		16
#define  MACH_MT_RB411U		17
#define  MACH_MT_RB493G		18
#define  MACH_MT_RB750G		19
#define  MACH_MT_RB435G		20
#define  MACH_MT_RB750		23
#define  MACH_MT_RB711		24
#define  MACH_MT_RB_OMNI	25
#define  MACH_MT_RB_SXT5D	26
#define  MACH_MT_RB_GROOVE	27
#define  MACH_MT_RB_OMNI_5FE	28	/* OmniTIK U-5HnD */
#define  MACH_MT_RB711R3	29
#define  MACH_MT_RB751G		30
#define  MACH_MT_RB711G		31
#define  MACH_MT_RB411L		33
#define  MACH_MT_RB750GL	35
#define  MACH_MT_RB411G		36
#define  MACH_MT_RB951		37
#define  MACH_MT_GROOVE52	38
#define  MACH_MT_RB2011G	39
#define  MACH_MT_RB912G		40
#define  MACH_MT_RB433GL	41
#define  MACH_MT_RB433UL	42
#define  MACH_MT_RB_SXTG	43
#define  MACH_MT_RB751		44
#define  MACH_MT_RB_GROOVE_5S	45
#define  MACH_MT_RB433L		46
#define  MACH_MT_RB411UL	47
#define  MACH_MT_RB711GT	48
#define  MACH_MT_RB2011L	49
#define  MACH_MT_RB2011LS	50
#define  MACH_MT_OMNITIK_WASP	51
#define  MACH_MT_RB951G		52
#define  MACH_MT_RB2011US	53
#define  MACH_MT_CCS226		55
#define  MACH_MT_CM2N           57
#define  MACH_MT_SXT2D		58
#define  MACH_MT_SXT5N		59
#define  MACH_MT_951HND		60
#define  MACH_MT_RB2011R5	61
#define  MACH_MT_RB953GS	62
#define  MACH_MT_CRS125G	63
#define  MACH_MT_SXTG5P		64
#define  MACH_MT_SXTG5C		65
#define  MACH_MT_RB922GS	66
#define  MACH_MT_mAP		67
#define  MACH_MT_CRS109		69
#define  MACH_MT_CCS112		70
#define  MACH_MT_SXT_LTE	71
#define  MACH_MT_CRS210		72
#define  MACH_MT_CRS212		73
#define  MACH_MT_RB911L		75
#define  MACH_MT_RB952		76
#define  MACH_MT_GROOVE_52AC	78
#define  MACH_MT_RB962		79
#define  MACH_MT_RB941HL	80
#define  MACH_MT_wAP		81
#define  MACH_MT_wAPG		82
#define  MACH_MT_RB750r2	83
#define  MACH_MT_RB960		84
#define  MACH_MT_RB750Gv2	85
#define  MACH_MT_mAPL		86
#define  MACH_MT_mAP2		87
#define  MACH_MT_LHG		88
#define  MACH_MT_SXT_AC		89
#define  MACH_MT_CRS106		90
#define  MACH_MT_QRTG		91
#define  MACH_MT_OMNI_SC	93
#define  MACH_MT_RB931		94
#define  MACH_MT_wAP_LTE	95
#define  MACH_MT_cAP		97
#define  MACH_MT_CCS112R4	98
#define  MACH_MT_RB750Gr3	99
#define  MACH_MT_M11		100
#define  MACH_MT_M33		101
#define  MACH_MT_wsAP		102
#define  MACH_MT_LHG_hb		104
#define  MACH_MT_LTAP_hb	105
#define  MACH_MT_PLC_hb		106
#define  MACH_MT_LHG60_hb	107
#define  MACH_MT_CRS105		108
#define  MACH_MT_SXTR_hb	109
#define  MACH_MT_CRS332		111
#define  MACH_MT_CRS312		112
#define  MACH_MT_CRS354		113
#define  MACH_MT_RB750Gr4	114

extern unsigned long mips_machgroup;
extern unsigned long mips_machtype;

extern int is_music_lite(void);
extern int is_wasp(void);
extern int is_honeybee(void);
extern int is_scorpion(void);

#endif
