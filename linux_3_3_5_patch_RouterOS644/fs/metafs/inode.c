#include <linux/module.h>
#include <linux/fs.h>
#include <linux/statfs.h>
#include <linux/pagemap.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/mount.h>
#include <asm/vm.h>

#define CMD_GETINODE		0
#define CMD_RELEASE_INODE	1
#define CMD_LOOKUP		2
#define CMD_READPAGE		3
#define CMD_READLINK		4
#define CMD_READDIR		5
#define CMD_WRITEPAGE		6
#define CMD_CREATE		7
#define CMD_UNLINK		8
#define CMD_SYMLINK		9
#define CMD_RENAME		10
#define CMD_SETINODE		11
#define CMD_STATFS		12
#define CMD_HLINK		13
#define CMD_FSYNC		14

struct hptime {
	unsigned sec;
	unsigned nsec;
};

struct getinode_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long ino;
} __attribute__((packed));

struct inode_rep {
	int status;
	unsigned long long ino;
	unsigned long long size;
	unsigned mode;
	unsigned nlink;
	unsigned uid;
	unsigned gid;
	unsigned rdev;
	struct hptime atime;
	struct hptime mtime;
	struct hptime ctime;
	unsigned long blksize;
	unsigned long long blocks;
} __attribute__((packed));

struct setinode_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long ino;
	unsigned long long size;
	unsigned mode;
	unsigned uid;
	unsigned gid;
	unsigned rdev;
} __attribute__((packed));

struct lookup_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long parent_ino;
	char name[0];
} __attribute__((packed));

struct create_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long parent_ino;
	unsigned mode;
	unsigned dev;
	char name[0];
} __attribute__((packed));

struct unlink_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long parent_ino;
	char name[0];
} __attribute__((packed));

struct symlink_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long parent_ino;
	unsigned namelen;
	char names[0];
} __attribute__((packed));

struct hlink_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long parent_ino;
	unsigned long long ino;
	char name[0];
} __attribute__((packed));

struct rename_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long old_parent_ino;
	unsigned long long new_parent_ino;
	unsigned old_namelen;
	char names[0];
} __attribute__((packed));

struct readpage_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long ino;
	unsigned long long offset;
	unsigned size;
} __attribute__((packed));

struct writepage_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long ino;
	unsigned long long offset;
	unsigned size;
} __attribute__((packed));

struct fsync_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long ino;
} __attribute__((packed));

struct readlink_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long ino;
} __attribute__((packed));

struct readlink_rep {
	int status;
	char target[0];
} __attribute__((packed));

struct readdir_req {
	unsigned short id;
	unsigned short cmd;
	unsigned long long ino;
	unsigned long long offset;
	unsigned size;
} __attribute__((packed));

struct dirnode {
	unsigned long long ino;
	unsigned long long offset;
	unsigned char type;
	unsigned short len;
	char name[0];
} __attribute__((packed));

struct readdir_rep {
	int status;
	unsigned long long offset;
	struct dirnode entries[0];
} __attribute__((packed));

struct statfs_req {
	unsigned short id;
	unsigned short cmd;
} __attribute__((packed));

struct statfs_rep {
	int status;
	unsigned blocks;
	unsigned bfree;
} __attribute__((packed));

#define BUF_COUNT	16

extern int vm_create_queue(unsigned id, unsigned irq,
			   unsigned tx, unsigned rx);
extern int vm_release_queue(unsigned id);

static volatile struct vdma_descr rx_descr[BUF_COUNT];
static volatile struct vdma_descr tx_descr[BUF_COUNT];

#define MFS_ID(sb) ((unsigned) sb->s_fs_info)


static void mfs_update_inode(struct inode *i, struct inode_rep *rep);
static struct inode *mfs_new_inode(struct super_block *sb,
				   struct inode_rep *rep);

static void start_new_request(unsigned *tx_idx, unsigned tx_slots,
			      unsigned *rx_idx, unsigned rx_slots)
{
	static DEFINE_MUTEX(mfs_lock);
	static unsigned cur_tx;
	static unsigned cur_rx;

	mutex_lock(&mfs_lock);

	*tx_idx = cur_tx;
	cur_tx += tx_slots;

	*rx_idx = cur_rx;
	cur_rx += rx_slots;

	mutex_unlock(&mfs_lock);
}

static void prepare_receive(unsigned idx, void *resp, unsigned rp_size)
{
	idx = idx & (BUF_COUNT - 1);

	rx_descr[idx].addr = (unsigned) resp;
	rx_descr[idx].size = rp_size;
}

static void post_request(unsigned idx, const void *req, unsigned rq_size)
{
	idx = idx & (BUF_COUNT - 1);

	while (tx_descr[idx].size & DONE) {
		hc_yield();
	}

	tx_descr[idx].addr = (unsigned) req;
	tx_descr[idx].size = rq_size | DONE;
}

static unsigned wait_for_reply(unsigned idx)
{
	idx = idx & (BUF_COUNT - 1);

	while (!(rx_descr[idx].size & DONE)) {
		hc_yield();
	}
	return rx_descr[idx].size & ~(PAGE_MASK<<1);
}

static unsigned send_request(const void *req, unsigned rq_size,
			     void *resp, unsigned rp_size)
{
	unsigned tx;
	unsigned rx;

	start_new_request(&tx, 1, &rx, 1);
	prepare_receive(rx, resp, rp_size);
	post_request(tx, req, rq_size);
	return wait_for_reply(rx);
}

static struct kmem_cache *mfs_inode_cachep;

static struct inode *mfs_alloc_inode(struct super_block *sb)
{
	return kmem_cache_alloc(mfs_inode_cachep, GFP_KERNEL);
}

static void mfs_destroy_inode(struct inode *inode)
{
	kmem_cache_free(mfs_inode_cachep, inode);
}

static struct dentry *mfs_lookup(struct inode *dir,
				 struct dentry *dentry, struct nameidata *nd)
{
	unsigned size = sizeof(struct lookup_req) + dentry->d_name.len;
	unsigned char buf[size];
	struct lookup_req *req = (struct lookup_req *) buf;
	struct inode_rep rep;
	struct inode *inode = NULL;
	struct dentry *res = NULL;
	unsigned ret;

	req->id = MFS_ID(dir->i_sb);
	req->cmd = CMD_LOOKUP;
	req->parent_ino = dir->i_ino;
	memcpy(req->name, dentry->d_name.name, dentry->d_name.len);

	rep.status = -EINVAL;
	ret = send_request(req, size, &rep, sizeof(rep));
	if (ret == sizeof(rep) && rep.status == 0)
		inode = mfs_new_inode(dir->i_sb, &rep);
	d_add(dentry, inode);
	return res;
}

static int mfs_create_file(struct inode *dir, struct dentry *dentry,
			   int mode, dev_t dev)
{
	unsigned size = sizeof(struct create_req) + dentry->d_name.len;
	unsigned char buf[size];
	struct create_req *req = (struct create_req *) buf;
	struct inode_rep rep;
	struct inode *inode = NULL;
	unsigned ret;

	req->id = MFS_ID(dir->i_sb);
	req->cmd = CMD_CREATE;
	req->parent_ino = dir->i_ino;
	req->mode = mode;
	req->dev = (unsigned) dev;
	memcpy(req->name, dentry->d_name.name, dentry->d_name.len);

	rep.status = -EINVAL;
	ret = send_request(req, size, &rep, sizeof(rep));
	if (ret < sizeof(rep))
		return rep.status;

	inode = mfs_new_inode(dir->i_sb, &rep);
	d_instantiate(dentry, inode);
	return 0;
}

static int mfs_create(struct inode *dir, struct dentry *dentry, umode_t mode,
		      struct nameidata *nd)
{
	return mfs_create_file(dir, dentry, mode, MKDEV(0, 0));
}

static int mfs_unlink(struct inode *dir, struct dentry *dentry)
{
	unsigned size = sizeof(struct unlink_req) + dentry->d_name.len;
	unsigned char buf[size];
	struct unlink_req *req = (struct unlink_req *) buf;
	int err = -EINVAL;
    
	req->id = MFS_ID(dir->i_sb);
	req->cmd = CMD_UNLINK;
	req->parent_ino = dir->i_ino;
	memcpy(req->name, dentry->d_name.name, dentry->d_name.len);

	send_request(req, size, &err, sizeof(err));
	return err;
}

static int mfs_symlink(struct inode *dir, struct dentry *dentry,
		       const char *target)
{
	unsigned tlen = strlen(target);
	unsigned size = sizeof(struct symlink_req) + dentry->d_name.len + tlen;
	unsigned char buf[size];
	struct symlink_req *req = (struct symlink_req *) buf;
	struct inode_rep rep;
	struct inode *inode = NULL;
	unsigned ret;
    
	req->id = MFS_ID(dir->i_sb);
	req->cmd = CMD_SYMLINK;
	req->parent_ino = dir->i_ino;
	req->namelen = dentry->d_name.len;
	memcpy(req->names, dentry->d_name.name, dentry->d_name.len);
	memcpy(req->names + req->namelen, target, tlen);

	rep.status = -EINVAL;
	ret = send_request(req, size, &rep, sizeof(rep));
	if (ret < sizeof(rep))
		return rep.status;

	inode = mfs_new_inode(dir->i_sb, &rep);
	d_instantiate(dentry, inode);
	return 0;
}

static int mfs_link(struct dentry *old_dentry, struct inode *dir,
		    struct dentry *dentry)
{
	unsigned size = sizeof(struct hlink_req) + dentry->d_name.len;
	unsigned char buf[size];
	struct hlink_req *req = (struct hlink_req *) buf;
	struct inode_rep rep;
	unsigned ret;
    
	req->id = MFS_ID(dir->i_sb);
	req->cmd = CMD_HLINK;
	req->parent_ino = dir->i_ino;
	req->ino = old_dentry->d_inode->i_ino;
	memcpy(req->name, dentry->d_name.name, dentry->d_name.len);

	rep.status = -EINVAL;
	ret = send_request(req, size, &rep, sizeof(rep));
	if (ret < sizeof(rep))
		return rep.status;

	mfs_update_inode(old_dentry->d_inode, &rep);

	atomic_inc(&old_dentry->d_inode->i_count);
	d_instantiate(dentry, old_dentry->d_inode);
	return 0;
}

static int mfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	return mfs_create_file(dir, dentry, mode | S_IFDIR, MKDEV(0, 0));
}

static int mfs_rmdir(struct inode *dir, struct dentry *dentry)
{
	return mfs_unlink(dir, dentry);
}

static int mfs_mknod(struct inode *dir, struct dentry *dentry, umode_t mode,
		     dev_t rdev) {
	return mfs_create_file(dir, dentry, mode, rdev);
}

static int mfs_rename(struct inode *old_dir, struct dentry *old_dentry,
		      struct inode *new_dir, struct dentry *new_dentry)
{
	unsigned size = sizeof(struct rename_req) +
	    old_dentry->d_name.len + new_dentry->d_name.len;
	unsigned char buf[size];
	struct rename_req *req = (struct rename_req *) buf;
	int err = -EINVAL;
    
	req->id = MFS_ID(old_dir->i_sb);
	req->cmd = CMD_RENAME;
	req->old_parent_ino = old_dir->i_ino;
	req->new_parent_ino = new_dir->i_ino;
	req->old_namelen = old_dentry->d_name.len;
	memcpy(req->names, old_dentry->d_name.name, old_dentry->d_name.len);
	memcpy(req->names + req->old_namelen,
	       new_dentry->d_name.name, new_dentry->d_name.len);

	send_request(req, size, &err, sizeof(err));
	return err;
}

static int mfs_readdir(struct file *file, void *dirent, filldir_t filldir)
{
	struct readdir_req req;
	struct readdir_rep *rep;
	struct dirnode *dn;
	unsigned len;
	int res = -EINVAL;

	rep = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!rep)
		return -ENOMEM;

	req.id = MFS_ID(file->f_dentry->d_inode->i_sb);
	req.cmd = CMD_READDIR;
	req.ino = file->f_dentry->d_inode->i_ino;
	req.offset = file->f_pos;
	req.size = PAGE_SIZE;

	len = send_request(&req, sizeof(req), rep, PAGE_SIZE);
	if (len <= sizeof(*rep)) {
		if (len >= sizeof(int))
			res = rep->status;
		goto eod;
	}
	
	dn = rep->entries;
	res = 0;
	while ((char *) dn + sizeof(struct dirnode) < (char *) rep + len) {
		if ((char *) dn + dn->len > (char *) rep + len)
			break;
		if (filldir(dirent, dn->name, dn->len - sizeof(struct dirnode),
			    dn->offset, dn->ino, dn->type) < 0)
			break;
		++res;
		dn = (struct dirnode *) ((unsigned char *) dn + dn->len);
	}
	file->f_pos = rep->offset;

  eod:
	kfree(rep);
	return res;
}

static int mfs_readpage(struct file *file, struct page *page)
{
	struct readpage_req req;
	void *buf;
	int res = -EIO;
	unsigned len;
	unsigned tx;
	unsigned rx;

	buf = kmap(page);
	if (!buf)
		goto err_out;

	req.id = MFS_ID(file->f_dentry->d_inode->i_sb);
	req.cmd = CMD_READPAGE;
	req.ino = file->f_dentry->d_inode->i_ino;
	req.offset = page_offset(page);
	req.size = PAGE_SIZE;

	start_new_request(&tx, 1, &rx, 2);
	prepare_receive(rx, &res, sizeof(res));
	prepare_receive(rx + 1, buf, PAGE_SIZE);
	post_request(tx, &req, sizeof(req));

	if (wait_for_reply(rx) < sizeof(res)) {
		res = -EINVAL;
		goto err_out;
	}
	if (res) {
		memset(buf, 0, PAGE_SIZE);
		SetPageError(page);
		goto err_buf;
	}
	len = wait_for_reply(rx + 1);

	memset(buf + len, 0, PAGE_SIZE - len);
	SetPageUptodate(page);

  err_buf:
	kunmap(page);
	flush_dcache_page(page);
  err_out:
	unlock_page(page);
	return res;
}

static int mfs_write_begin(struct file *file, struct address_space *mapping,
			   loff_t pos, unsigned len, unsigned flags,
			   struct page **pagep, void **fsdata)
{
	pgoff_t index = pos >> PAGE_CACHE_SHIFT;

	*pagep = grab_cache_page_write_begin(mapping, index, flags);
	if (!*pagep)
		return -ENOMEM;

	// FIXME: do prereading

	return 0;
}

static int mfs_write_end(struct file *file, struct address_space *mapping,
			 loff_t pos, unsigned blen, unsigned copied,
			 struct page *page, void *fsdata)
{
	struct inode *i = file->f_dentry->d_inode;
	struct writepage_req req;
	void *buf;
	int len = -EFAULT;
	unsigned tx;
	unsigned rx;
	unsigned size;

	flush_dcache_page(page);

	buf = kmap(page);
	if (!buf)
		return -EINVAL;

	req.id = MFS_ID(i->i_sb);
	req.cmd = CMD_WRITEPAGE;
	req.ino = file->f_dentry->d_inode->i_ino;
	req.offset = pos;
	req.size = blen;

	start_new_request(&tx, 2, &rx, 1);
	prepare_receive(rx, &len, sizeof(len));
	post_request(tx, &req, sizeof(req));
	post_request(tx + 1, buf + (pos & (PAGE_CACHE_SIZE - 1)), blen);
	wait_for_reply(rx);

	if (len >= 0) {
		if (len != blen) {
			SetPageError(page);
			ClearPageUptodate(page);
		} else {
			SetPageUptodate(page);
		}

		size = req.offset + len;
		if (size > i_size_read(i)) i_size_write(i, size);
	}

	kunmap(page);
	unlock_page(page);
	page_cache_release(page);
	return len;
}

static int mfs_fsync(struct file *file, loff_t start, loff_t end, int datasync)
{
 	struct inode *inode = file->f_mapping->host;
	struct fsync_req req;
	int err;

	err = filemap_write_and_wait_range(inode->i_mapping, start, end);
	if (err)
		return err;

	req.id = MFS_ID(file->f_mapping->host->i_sb);
	req.cmd = CMD_FSYNC;
	req.ino = file->f_mapping->host->i_ino;

	err = -EINVAL;
	send_request(&req, sizeof(req), &err, sizeof(err));
	return err;
}

static void *mfs_follow_link(struct dentry *dentry, struct nameidata *nd)
{
	struct readlink_req req;
	struct readlink_rep *rep;
	int len;

	rep = kmalloc(256, GFP_KERNEL);
	if (!rep)
		return ERR_PTR(-ENOMEM);

	req.id = MFS_ID(dentry->d_inode->i_sb);
	req.cmd = CMD_READLINK;
	req.ino = dentry->d_inode->i_ino;
	
	rep->status = -EINVAL;
	len = send_request(&req, sizeof(req), rep, 255);
	if (len < sizeof(*rep) + 1) {
		kfree(rep);
		return ERR_PTR(rep->status);
	}

	*((char *) rep + len) = 0;
	nd_set_link(nd, rep->target);
	return NULL;
}

static void mfs_put_link(struct dentry *direntry,
			 struct nameidata *nd, void *cookie)
{
	char *p = nd_get_link(nd);

	if (!IS_ERR(p))
		kfree(p - sizeof(struct readlink_rep));
}

static int mfs_setattr(struct dentry *dentry, struct iattr *attr)
{
	struct setinode_req req;
	struct inode_rep rep;
	struct inode *i = dentry->d_inode;
	unsigned ia = attr->ia_valid;
	unsigned len;

	req.id = MFS_ID(i->i_sb);
	req.cmd = CMD_SETINODE;
	req.ino = i->i_ino;
	req.mode = ia & ATTR_MODE ? attr->ia_mode : i->i_mode;
	req.uid = ia & ATTR_UID ? attr->ia_uid : i->i_uid;
	req.gid = ia & ATTR_GID ? attr->ia_gid : i->i_gid;
	req.size = ia & ATTR_SIZE ? attr->ia_size : i->i_size;

	len = send_request(&req, sizeof(req), &rep, sizeof(rep));
	if (len < sizeof(rep))
		return -EINVAL;
	    
	if (rep.status)
		return rep.status;

	mfs_update_inode(i, &rep);
	return 0;
}

static const struct file_operations mfs_dir_fops = {
	.read		= generic_read_dir,
	.readdir	= mfs_readdir,
};

static const struct inode_operations mfs_dir_ops = {
	.lookup		= mfs_lookup,
	.create		= mfs_create,
	.link		= mfs_link,
	.unlink		= mfs_unlink,
	.symlink	= mfs_symlink,
	.mkdir		= mfs_mkdir,
	.rmdir		= mfs_rmdir,
	.mknod		= mfs_mknod,
	.rename		= mfs_rename,
	.setattr	= mfs_setattr,
};

static const struct inode_operations mfs_file_ops = {
	.setattr	= mfs_setattr,
};

static const struct file_operations mfs_fops = {
	.llseek		= generic_file_llseek,
	.read		= do_sync_read,
	.write		= do_sync_write,
	.aio_read	= generic_file_aio_read,
	.aio_write	= generic_file_aio_write,
	.mmap		= generic_file_readonly_mmap,
	.splice_read	= generic_file_splice_read,
	.fsync		= mfs_fsync,
};

static const struct address_space_operations mfs_aops = {
	.readpage	= mfs_readpage,
	.write_begin	= mfs_write_begin,
	.write_end	= mfs_write_end,
};

static const struct inode_operations mfs_link_ops = {
	.readlink	= generic_readlink,
	.follow_link	= mfs_follow_link,
	.put_link	= mfs_put_link,
	.setattr	= mfs_setattr,
};

static void mfs_update_inode(struct inode *i, struct inode_rep *rep)
{
	i->i_ino = rep->ino;
	i->i_mode = rep->mode;
	set_nlink(i, rep->nlink);
	i->i_uid = rep->uid;
	i->i_gid = rep->gid;
	i->i_size = rep->size;
	i->i_atime.tv_sec = rep->atime.sec;
	i->i_atime.tv_nsec = rep->atime.nsec;
	i->i_mtime.tv_sec = rep->mtime.sec;
	i->i_mtime.tv_nsec = rep->mtime.nsec;
	i->i_ctime.tv_sec = rep->ctime.sec;
	i->i_ctime.tv_nsec = rep->ctime.nsec;
	i->i_blkbits = ffs(rep->blksize);
	i->i_blocks = rep->blocks;

	if (i->i_sb->s_flags & MS_RDONLY)
		i->i_mode &= ~0222;
}

static struct inode *mfs_new_inode(struct super_block *sb,
				   struct inode_rep *rep)
{
	struct inode *i = new_inode(sb);
	if (!i) return NULL;

	mfs_update_inode(i, rep);

	if (S_ISREG(rep->mode)) {
		i->i_op = &mfs_file_ops;
		i->i_fop = &mfs_fops;
		i->i_data.a_ops = &mfs_aops;
	} else if (S_ISDIR(rep->mode)) {
		i->i_op = &mfs_dir_ops;
		i->i_fop = &mfs_dir_fops;
	} else if (S_ISLNK(rep->mode)) {
		i->i_op = &mfs_link_ops;
	} else {
		init_special_inode(i, rep->mode, (dev_t) rep->rdev);
	}

	insert_inode_hash(i);
	return i;
}

static struct inode *mfs_getinode(struct super_block *sb,
				  unsigned long long ino)
{
	struct getinode_req req;
	struct inode_rep rep;
	unsigned len;

	req.id = MFS_ID(sb);
	req.cmd = CMD_GETINODE;
	req.ino = ino;
	len = send_request(&req, sizeof(req), &rep, sizeof(rep));

	if (len < sizeof(rep) || rep.status)
		return NULL;

	return mfs_new_inode(sb, &rep);
}

static void mfs_put_super(struct super_block *sb)
{
}

static int mfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	struct statfs_req req;
	struct statfs_rep rep;
	struct super_block *sb = dentry->d_sb;
	unsigned len;

	req.id = MFS_ID(sb);
	req.cmd = CMD_STATFS;
	rep.status = -EINVAL;
	len = send_request(&req, sizeof(req), &rep, sizeof(rep));

	if (len < sizeof(rep) || rep.status)
		return rep.status;

	buf->f_type = sb->s_magic;
	buf->f_bsize = 512;
	buf->f_blocks = rep.blocks;
	buf->f_bfree = rep.bfree;
	buf->f_bavail = rep.bfree;
	buf->f_namelen = 255;

	return 0;
}

static const struct super_operations mfs_ops = {
	.alloc_inode	= mfs_alloc_inode,
	.destroy_inode	= mfs_destroy_inode,
	.put_super	= mfs_put_super,
	.statfs		= mfs_statfs,
};

static int mfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *root;
	unsigned id;

	if (*(char *) data == '/') ++data;
	id = simple_strtoul((char *) data, NULL, 10);

	sb->s_magic = 0xdeadbeef;
	sb->s_op = &mfs_ops;
	if (id == 0)
		sb->s_flags |= MS_RDONLY;
	sb->s_fs_info = (void *) id;

	root = mfs_getinode(sb, 0);
	if (!root)	    
		goto out;

	sb->s_root = d_alloc_root(root);
	if (!sb->s_root)
		goto outiput;

	return 0;

  outiput:
	iput(root);
  out:
	return -EINVAL;
}

static struct dentry *mfs_mount(struct file_system_type *fs_type, int flags,
				const char *dev_name, void *data)
{
	return mount_nodev(fs_type, flags, (void *) dev_name, mfs_fill_super);
}

static struct file_system_type mfs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "metafs",
	.mount		= mfs_mount,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

static void init_once(void *foo)
{
	struct inode * inode = (struct inode *) foo;

	inode_init_once(inode);
}

static int __init init_mfs_fs(void)
{
	unsigned i;
	int err;

	if (vm_running() != 0)
		return 0;

	printk("MFS init\n");
	mfs_inode_cachep = kmem_cache_create("metafs_inode_cache",
					     sizeof(struct inode),
					     0, (SLAB_RECLAIM_ACCOUNT|
						 SLAB_MEM_SPREAD),
					     init_once);
	if (!mfs_inode_cachep)
		return -ENOMEM;

	for (i = 0; i < BUF_COUNT; ++i) {
		tx_descr[i].addr = 0;
		tx_descr[i].size = 0;
		tx_descr[i].next = (unsigned) &tx_descr[i + 1];

		rx_descr[i].addr = 0;
		rx_descr[i].size = DONE;
		rx_descr[i].next = (unsigned) &rx_descr[i + 1];
	}
	tx_descr[BUF_COUNT - 1].next = (unsigned) &tx_descr[0];
	rx_descr[BUF_COUNT - 1].next = (unsigned) &rx_descr[0];
	
	vm_create_queue(2, -1u,
			(unsigned) &tx_descr[0], (unsigned) &rx_descr[0]);

        err = register_filesystem(&mfs_fs_type);
	if (err != 0) {
		kmem_cache_destroy(mfs_inode_cachep);
		return err;
	}

	return 0;
}

static void __exit exit_mfs_fs(void)
{
	unregister_filesystem(&mfs_fs_type);
	kmem_cache_destroy(mfs_inode_cachep);
}

module_init(init_mfs_fs);
module_exit(exit_mfs_fs);
