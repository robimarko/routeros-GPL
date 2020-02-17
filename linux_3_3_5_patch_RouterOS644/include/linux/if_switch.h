#ifndef _LINUX_IF_SWITCH_H
#define _LINUX_IF_SWITCH_H

#include <linux/types.h>

// only 16 ioctls available
#define SIOCGTYPE	(SIOCDEVPRIVATE + 0)
#define SIOCGSWITCH	(SIOCDEVPRIVATE + 1)
#define SIOCSTXBW	(SIOCDEVPRIVATE + 2)
#define SIOCSRXBW	(SIOCDEVPRIVATE + 3)
#define SIOCSSWITCHED	(SIOCDEVPRIVATE + 4)
#define SIOCSMIRROR	(SIOCDEVPRIVATE + 5)
#define SIOCGREG	(SIOCDEVPRIVATE + 8)
#define SIOCSREG	(SIOCDEVPRIVATE + 9)
#define SIOCGPORT	(SIOCDEVPRIVATE + 10)
#define SIOCGCONFIG	(SIOCDEVPRIVATE + 11)
#define SIOCSCONFIG	(SIOCDEVPRIVATE + 12)
#define SIOCBRIDGE 	(SIOCDEVPRIVATE + 14)
#define SIOCSWICHMAX	(SIOCDEVPRIVATE + 15)

struct switch_params {
    unsigned data;
    unsigned data2;
};

struct switch_fdb_params {
    __u16 vid;
    __u8 mac[6];
    __u8 add;
};

struct switch_bridge_params {
   __u32 flags;
   __u32 tpid;
};

#define SWITCH_ADMTEK		0
#define SWITCH_ICPLUS175C	1

#define MIRROR_NONE -1u
#define MIRROR_CPU -2u

#define SWITCH_BR_SET_STP 1
#define SWITCH_BR_SET_BRIDGE 2
#define SWITCH_BR_FLUSH 3
#define SWITCH_BR_ADD_VLAN 4
#define SWITCH_BR_DEL_VLAN 5
#define SWITCH_BR_SET_VLAN_OPTS 6
#define SWITCH_BR_SET_MSTI 7
#define SWITCH_BR_GET_BOND_ID 8
#define SWITCH_BR_GET_BOND_LID 9
#define SWITCH_BR_ADD_FDB 10
#define SWITCH_BR_SET_PORT_OPTS 11
#define SWITCH_BR_SET_AGING 12
#define SWITCH_BR_SET_FWD_MASK 13

#define STP_DISABLE 0
#define STP_BLOCK   1
#define STP_LEARN   2
#define STP_FORWARD 3

// see if_bridge.h
#define SW_VLAN_PVID BIT(1)
#define SW_VLAN_UNTAGGED BIT(2)
#define SW_VLAN_FORCE BIT(3)

#endif
