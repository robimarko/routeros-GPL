#ifndef MT_RB500_H
#define MT_RB500_H

#define RB500_GPIO_NUM_NANDRDY	8
#define RB500_GPIO_NUM_CFRDY	13

#define RB500_GPIO_NANDRDY	(1 << RB500_GPIO_NUM_NANDRDY)
#define RB500_GPIO_CFRDY	(1 << RB500_GPIO_NUM_CFRDY)

/* latch U5 bits, depends on revision */
#define RB500_LO_WPX	(1 << 0)
#define RB500_LO_NCE	(1 << 0)	// rev5 only
#define RB500_LO_ALE	(1 << 1)
#define RB500_LO_CLE	(1 << 2)
#define RB500_LO_BEEP	(1 << 3)	// rev5 only
#define RB500_LO_FOFF	(1 << 5)
#define RB500_LO_SPICS	(1 << 6)
#define RB500_LO_ULED	(1 << 7)

extern void changeLatchU5(unsigned char orMask, unsigned char nandMask);
extern void rb500_beep(unsigned freq);

#endif
