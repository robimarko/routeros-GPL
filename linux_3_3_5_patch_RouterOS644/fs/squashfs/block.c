/*
 * Squashfs - a compressed read only filesystem for Linux
 *
 * Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008
 * Phillip Lougher <phillip@squashfs.org.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * block.c
 */

/*
 * This file implements the low-level routines to read and decompress
 * datablocks and metadata blocks.
 */

#include <linux/fs.h>
#include <linux/vfs.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/string.h>
#include <linux/buffer_head.h>

#include "squashfs_fs.h"
#include "squashfs_fs_sb.h"
#include "squashfs.h"
#include "decompressor.h"

static struct page *page_read(struct super_block *sb, pgoff_t index)
{
	struct squashfs_sb_info *msblk = sb->s_fs_info;
	struct file *file = msblk->file;
	struct address_space *mapping = file->f_mapping;
	struct page *page;

	page = find_or_create_page(mapping, index + msblk->offset, GFP_NOFS);
	if (!page)
		return NULL;

	if (!PageUptodate(page)) {
		if (mapping->a_ops->readpage(file, page))
			goto err;
		wait_on_page_locked(page);
		lock_page(page);
		if (!PageUptodate(page))
			goto err;
	}
	return page;

  err:
	unlock_page(page);
	page_cache_release(page);
	return NULL;
}

/*
 * Read the metadata block length, this is stored in the first two
 * bytes of the metadata block.
 */
static struct page *get_block_length(struct super_block *sb,
			u64 *cur_index, int *offset, int *length)
{
	struct squashfs_sb_info *msblk = sb->s_fs_info;
	struct page *pg;

	pg = page_read(sb, *cur_index);
	if (pg == NULL)
		return NULL;

	if (msblk->devblksize - *offset == 1) {
		*length = ((unsigned char *) page_address(pg))[*offset];

		unlock_page(pg);
		page_cache_release(pg);

		pg = page_read(sb, ++(*cur_index));
		if (pg == NULL)
			return NULL;
		*length |= ((unsigned char *) page_address(pg))[0] << 8;
		*offset = 1;
	} else {
		*length = ((unsigned char *) page_address(pg))[*offset] |
			((unsigned char *) page_address(pg))[*offset + 1] << 8;
		*offset += 2;

		if (*offset == msblk->devblksize) {
			unlock_page(pg);
			page_cache_release(pg);

			pg = page_read(sb, ++(*cur_index));
			if (pg == NULL)
				return NULL;
			*offset = 0;
		}
	}

	return pg;
}

/*
 * Read and decompress a metadata block or datablock.  Length is non-zero
 * if a datablock is being read (the size is stored elsewhere in the
 * filesystem), otherwise the length is obtained from the first two bytes of
 * the metadata block.  A bit in the length field indicates if the block
 * is stored uncompressed in the filesystem (usually because compression
 * generated a larger block - this does occasionally happen with zlib).
 */
int squashfs_read_data(struct super_block *sb, void **buffer, u64 index,
			int length, u64 *next_index, int srclength, int pages)
{
	struct squashfs_sb_info *msblk = sb->s_fs_info;
	struct page **pgs;
	int offset = index & ((1 << msblk->devblksize_log2) - 1);
	u64 cur_index = index >> msblk->devblksize_log2;
	int bytes, compressed, b = 0, k = 0, page = 0, avail;

	pgs = kcalloc(((srclength + msblk->devblksize - 1)
		>> msblk->devblksize_log2) + 1, sizeof(*pgs), GFP_KERNEL);
	if (pgs == NULL)
		return -ENOMEM;

	if (length) {
		/*
		 * Datablock.
		 */
		bytes = -offset;
		compressed = SQUASHFS_COMPRESSED_BLOCK(length);
		length = SQUASHFS_COMPRESSED_SIZE_BLOCK(length);
		if (next_index)
			*next_index = index + length;

		TRACE("Block @ 0x%llx, %scompressed size %d, src size %d\n",
			index, compressed ? "" : "un", length, srclength);

		if (length < 0 || length > srclength ||
				(index + length) > msblk->bytes_used)
			goto read_failure;

		for (b = 0; bytes < length; b++, cur_index++) {
		    
			pgs[b] = page_read(sb, cur_index);
			if (pgs[b] == NULL)
				goto read_failure;
			bytes += msblk->devblksize;
		}
	} else {
		/*
		 * Metadata block.
		 */
		if ((index + 2) > msblk->bytes_used)
			goto read_failure;

		pgs[0] = get_block_length(sb, &cur_index, &offset, &length);
		if (pgs[0] == NULL)
			goto read_failure;
		b = 1;

		bytes = msblk->devblksize - offset;
		compressed = SQUASHFS_COMPRESSED(length);
		length = SQUASHFS_COMPRESSED_SIZE(length);
		if (next_index)
			*next_index = index + length + 2;

		TRACE("Block @ 0x%llx, %scompressed size %d\n", index,
				compressed ? "" : "un", length);

		if (length < 0 || length > srclength ||
					(index + length) > msblk->bytes_used)
			goto read_failure;

		for (; bytes < length; b++) {
			pgs[b] = page_read(sb, ++cur_index);
			if (pgs[b] == NULL)
				goto read_failure;
			bytes += msblk->devblksize;
		}
	}

	if (compressed) {
		k = 0;
		length = squashfs_decompress(msblk, buffer, pgs, b, offset,
			 length, srclength, pages);
		if (length < 0)
			goto read_failure;
	} else {
		/*
		 * Block is uncompressed.
		 */
		int in, pg_offset = 0;

		for (bytes = length; k < b; k++) {
			in = min(bytes, msblk->devblksize - offset);
			bytes -= in;
			while (in) {
				if (pg_offset == PAGE_CACHE_SIZE) {
					page++;
					pg_offset = 0;
				}
				avail = min_t(int, in, PAGE_CACHE_SIZE -
						pg_offset);
				memcpy(buffer[page] + pg_offset,
				       page_address(pgs[k]) + offset, avail);
				in -= avail;
				pg_offset += avail;
				offset += avail;
			}
			offset = 0;
		}
	}

  end:
	for (k = 0; k < b; ++k) {
		unlock_page(pgs[k]);
		page_cache_release(pgs[k]);
	}

	kfree(pgs);
	return length;

read_failure:
	ERROR("squashfs_read_data failed to read block 0x%llx\n",
					(unsigned long long) index);
	length = -EIO;
	goto end;
}
