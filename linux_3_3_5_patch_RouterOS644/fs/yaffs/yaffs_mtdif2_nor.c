#include "yportenv.h"


#include "yaffs_mtdif2_nor.h"

#include "linux/mtd/mtd.h"
#include "linux/types.h"
#include "linux/time.h"

#include "yaffs_packedtags2.h"

#define NOR_OOB_SIZE 16
#define NOR_PAGE_SIZE(x) (x + NOR_OOB_SIZE)

static inline loff_t chunkToAddr(yaffs_Device *dev, struct mtd_info *mtd,
                int chunkInNAND)
{
        return MTD_NOR_ERASESIZE * (chunkInNAND / dev->nChunksPerBlock)
                + NOR_PAGE_SIZE(dev->nBytesPerChunk)
                * (chunkInNAND % dev->nChunksPerBlock);
}

int normtd2_WriteChunkWithTagsToNAND(yaffs_Device * dev, int chunkInNAND,
				      const __u8 * data,
				      const yaffs_ExtendedTags * tags)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
        size_t dummy;
	int retval = 0;

	loff_t addr = chunkToAddr(dev, mtd, chunkInNAND);

	yaffs_PackedTags2 pt;

	T(YAFFS_TRACE_MTD,
	  (TSTR
	   ("normtd2_WriteChunkWithTagsToNAND chunk %d addr 0x%08llx data %p tags %p"
	    TENDSTR), chunkInNAND, addr, data, tags));

	if (data && tags) {
                // Calculates ECC, but doesn't write it, return NOR_OOB_SIZE
                yaffs_PackTags2(&pt, tags, NOR_OOB_SIZE);

                retval = mtd->write(mtd, addr, dev->nBytesPerChunk,
                                &dummy, data);
                if (retval == 0) {
                        retval = mtd->write(mtd, addr + dev->nBytesPerChunk,
                                        NOR_OOB_SIZE, &dummy, (__u8 *) &pt);
                }
	} else
		BUG(); /* both tags and data should always be present */

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int normtd2_ReadChunkWithTagsFromNAND(yaffs_Device * dev, int chunkInNAND,
				       __u8 * data, yaffs_ExtendedTags * tags)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	size_t dummy;
	int retval = 0;

	loff_t addr = chunkToAddr(dev, mtd, chunkInNAND);

	yaffs_PackedTags2 pt;

	T(YAFFS_TRACE_MTD,
	  (TSTR
	   ("normtd2_ReadChunkWithTagsFromNAND chunk %d addr 0x%08llx data %p tags %p"
	    TENDSTR), chunkInNAND, addr, data, tags));

	if (data) {
		retval = mtd->read(mtd, addr, dev->nBytesPerChunk, &dummy, data);
        }
        if (tags && retval == 0) {
		retval = mtd->read(mtd, addr + dev->nBytesPerChunk,
                                NOR_OOB_SIZE, &dummy, dev->spareBuffer);
                memcpy(&pt, dev->spareBuffer, sizeof(pt));
	}

	if (tags)
		yaffs_UnpackTags2(tags, &pt, 1); // Ignore ecc

	if (retval == -EUCLEAN) {
		if (tags) tags->eccResult = YAFFS_ECC_RESULT_FIXED;
		retval = 0;
	}

	if (tags && retval == -EBADMSG && tags->eccResult == YAFFS_ECC_RESULT_NO_ERROR)
		tags->eccResult = YAFFS_ECC_RESULT_UNFIXED;

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int normtd2_MarkNANDBlockBad(struct yaffs_DeviceStruct *dev, int blockNo)
{
        return YAFFS_OK;
}

int normtd2_QueryNANDBlock(struct yaffs_DeviceStruct *dev, int blockNo,
			    yaffs_BlockState * state, int *sequenceNumber)
{
	int retval = 0;
        yaffs_ExtendedTags t;

	T(YAFFS_TRACE_MTD,
	  (TSTR("normtd2_QueryNANDBlock %d" TENDSTR), blockNo));

        retval = normtd2_ReadChunkWithTagsFromNAND(dev,
                        blockNo *
                        dev->nChunksPerBlock, NULL,
                        &t);

        if (t.chunkUsed) {
                *sequenceNumber = t.sequenceNumber;
                *state = YAFFS_BLOCK_STATE_NEEDS_SCANNING;
        } else {
                *sequenceNumber = 0;
                *state = YAFFS_BLOCK_STATE_EMPTY;
        }

        T(YAFFS_TRACE_MTD,
                (TSTR("block is bad seq %d state %d" TENDSTR), *sequenceNumber,
                 *state));

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int normtd_EraseBlockInNAND(yaffs_Device * dev, int blockNumber)
{
	struct mtd_info *mtd = (struct mtd_info *)(dev->genericDevice);
	__u32 addr = ((loff_t) blockNumber) * MTD_NOR_ERASESIZE;
	struct erase_info ei;
	int retval = 0;

	ei.mtd = mtd;
	ei.addr = addr;
	ei.len = MTD_NOR_ERASESIZE;
	ei.time = 1000;
	ei.retries = 2;
	ei.callback = NULL;
	ei.priv = (u_long) dev;

	/* Todo finish off the ei if required */

        T(YAFFS_TRACE_MTD,
                (TSTR("normtd_EraseBlockInNAND block %d, addr 0x%08x, len %lld"
                      TENDSTR), blockNumber, addr, ei.len));

	retval = mtd->erase(mtd, &ei);

	if (retval == 0)
		return YAFFS_OK;
	else
		return YAFFS_FAIL;
}

int normtd_InitialiseNAND(yaffs_Device * dev)
{
	return YAFFS_OK;
}

