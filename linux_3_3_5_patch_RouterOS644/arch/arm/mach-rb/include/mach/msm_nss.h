/* * Copyright (c) 2013 Qualcomm Atheros, Inc. * */

/*
 * Qualcomm NSS Header file
 *
 */

#ifndef __MSM_NSS__H
#define __MSM_NSS__H

/*
 * nss_feature_enabled
 */
enum nss_feature_enabled {
	NSS_FEATURE_NOT_ENABLED = 0,	/* Feature is not enabled on this core */
	NSS_FEATURE_ENABLED,		/* Feature is enabled on this core */
};

/*
 * nss_platform_data
 *	Platform data per core
 */
struct nss_platform_data {
	uint32_t num_irq;	/* No. of interrupts supported per core */
	uint32_t irq[2];	/* IRQ numbers per interrupt */
	uint32_t nmap;		/* Virtual address of NSS CSM space */
	uint32_t vmap;		/* Virtual address of NSS virtual register map */
	uint32_t nphys;		/* Physical address of NSS CSM space */
	uint32_t vphys;		/* Physical address of NSS virtual register map */
	uint32_t rst_addr;	/* Reset address of NSS core */
	enum nss_feature_enabled ipv4_enabled;	/* Does this core handle IPv4? */
	enum nss_feature_enabled ipv6_enabled;	/* Does this core handle IPv6? */
	enum nss_feature_enabled l2switch_enabled;	/* Does this core handle L2 switch? */
	enum nss_feature_enabled crypto_enabled;	/* Does this core handle crypto? */
	enum nss_feature_enabled ipsec_enabled;	/* Does this core handle IPsec? */
	enum nss_feature_enabled wlan_enabled;	/* Does this core handle WLAN 11ac? */
	enum nss_feature_enabled tun6rd_enabled;	/* Does this core handle 6rd Tunnel ? */
	enum nss_feature_enabled gmac_enabled[4];	/* Does this core handle GMACs? */
};

#endif /* __MSM_NSS__H */
