#ifndef __YAFFS_MTDIF2_NOR_H__
#define __YAFFS_MTDIF2_NOR_H__

#include "yaffs_guts.h"
#define MTD_NOR_ERASESIZE	0x10000 // 64k

int normtd2_WriteChunkWithTagsToNAND(yaffs_Device * dev, int chunkInNAND,
				      const __u8 * data,
				      const yaffs_ExtendedTags * tags);
int normtd2_ReadChunkWithTagsFromNAND(yaffs_Device * dev, int chunkInNAND,
				       __u8 * data, yaffs_ExtendedTags * tags);
int normtd2_MarkNANDBlockBad(struct yaffs_DeviceStruct *dev, int blockNo);
int normtd2_QueryNANDBlock(struct yaffs_DeviceStruct *dev, int blockNo,
			    yaffs_BlockState * state, int *sequenceNumber);
int normtd_EraseBlockInNAND(yaffs_Device * dev, int blockNumber);
int normtd_InitialiseNAND(yaffs_Device * dev);

#endif
