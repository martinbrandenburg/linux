// SPDX-License-Identifier: GPL-2.0
/*
 * (C) 2001 Clemson University and The University of Chicago
 * Copyright 2018 Omnibond Systems, L.L.C.
 *
 * See COPYING in top-level directory.
 */

/*
 *  Linux VFS inode operations.
 */

#include <linux/bvec.h>
#include "orangefs-kernel.h"
#include "orangefs-bufmap.h"
#include "orangefs-trace.h"

static int orangefs_writepage_locked(struct page *page,
    struct writeback_control *wbc)
{
	struct inode *inode = page->mapping->host;
	struct orangefs_write_request *wr;
	struct iov_iter iter;
	struct bio_vec bv;
	size_t len, wlen;
	ssize_t ret;
	loff_t off;

	set_page_writeback(page);

	if (PagePrivate(page)) {
		wr = (struct orangefs_write_request *)page_private(page);
		BUG_ON(!wr);
		if (wr->mwrite) {
			off = page_offset(page);
			len = i_size_read(inode);
			if (off + PAGE_SIZE > len)
				wlen = len - off;
			else
				wlen = PAGE_SIZE;
		} else {
			off = wr->pos;
			wlen = wr->len;
			len = i_size_read(inode);
		}
	} else {
/*		BUG();*/
		/* It's not private so there's nothing to write, right? */
		printk("writepage not private!\n");
		end_page_writeback(page);
		return 0;

	}

	trace_orangefs_writepage(off, wlen, wr->mwrite);

	bv.bv_page = page;
	bv.bv_len = wlen;
	bv.bv_offset = 0;
	iov_iter_bvec(&iter, ITER_BVEC | WRITE, &bv, 1, wlen);

	ret = wait_for_direct_io(ORANGEFS_IO_WRITE, inode, &off, &iter, wlen,
	    len, wr);
	if (ret < 0) {
		SetPageError(page);
		mapping_set_error(page->mapping, ret);
	} else {
		ret = 0;
		if (wr) {
			ClearPagePrivate(page);
			kfree(wr);
		}
	}
	end_page_writeback(page);
	return ret;
}

static int do_writepage_if_necessary(struct page *page, loff_t pos,
    unsigned len)
{
	struct orangefs_write_request *wr;
	struct writeback_control wbc = {
		.sync_mode = WB_SYNC_ALL,
		.nr_to_write = 0,
	};
	int r;
	if (PagePrivate(page)) {
		int noncontig;
		wr = (struct orangefs_write_request *)page_private(page);
		BUG_ON(!wr);
 		noncontig = pos + len < wr->pos || wr->pos + wr->len < pos;
		/*
		 * If the new request is not contiguous with the last one or if
		 * the uid or gid is different, the page must be written out
		 * before continuing.
		 */
		if (noncontig ||
		    !uid_eq(current_fsuid(), wr->uid) ||
		    !gid_eq(current_fsgid(), wr->gid)) {
			wbc.range_start = page_file_offset(page);
			wbc.range_end = wbc.range_start + PAGE_SIZE - 1;
			wait_on_page_writeback(page);
			if (clear_page_dirty_for_io(page)) {
				trace_orangefs_early_writeback(noncontig ?
				    1 : 2);
				r = orangefs_writepage_locked(page, &wbc);
				if (r)
					return r;
			}
			BUG_ON(PagePrivate(page));
		}
	}
	return 0;
}

static int update_wr(struct page *page, loff_t pos, unsigned len, int mwrite)
{
	struct orangefs_write_request *wr;
	if (PagePrivate(page)) {
		wr = (struct orangefs_write_request *)page_private(page);
		BUG_ON(!wr);
		if (mwrite) {
			wr->mwrite = 1;
			return 0;
		}
		if (pos < wr->pos) {
			wr->len += wr->pos - pos;
			wr->pos = pos;
		}
		if (pos + len > wr->pos + wr->len)
			wr->len = pos + len - wr->pos;
		else
			wr->len = wr->pos + wr->len - wr->pos;
	} else {
		wr = kmalloc(sizeof *wr, GFP_KERNEL);
		if (wr) {
			wr->pos = pos;
			wr->len = len;
			wr->uid = current_fsuid();
			wr->gid = current_fsgid();
			wr->mwrite = mwrite;
			SetPagePrivate(page);
			set_page_private(page, (unsigned long)wr);
		} else {
			return -ENOMEM;
		}
	}
	return 0;
}

int orangefs_page_mkwrite(struct vm_fault *vmf)
{
	struct page *page = vmf->page;
	struct inode *inode = file_inode(vmf->vma->vm_file);
	unsigned len;
	int r;

	/* Do not write past the file size. */
	len = i_size_read(inode) - page_file_offset(page);
	if (len > PAGE_SIZE)
		len = PAGE_SIZE;

	lock_page(page);
	r = do_writepage_if_necessary(page, page_file_offset(page),
	    len);
	if (r) {
		r = VM_FAULT_RETRY;
		unlock_page(vmf->page);
		return r;
	}
	r = update_wr(page, page_file_offset(page), len, 1);
	if (r) {
		r = VM_FAULT_RETRY;
		unlock_page(vmf->page);
		return r;
	}

	r = VM_FAULT_LOCKED;
	sb_start_pagefault(inode->i_sb);
	file_update_time(vmf->vma->vm_file);
	if (page->mapping != inode->i_mapping) {
		unlock_page(page);
		r = VM_FAULT_NOPAGE;
		goto out;
	}
	/*
	 * We mark the page dirty already here so that when freeze is in
	 * progress, we are guaranteed that writeback during freezing will
	 * see the dirty page and writeprotect it again.
	 */
	set_page_dirty(page);
	wait_for_stable_page(page);
out:
	sb_end_pagefault(inode->i_sb);
	return r;
}

static int orangefs_writepage(struct page *page, struct writeback_control *wbc)
{
	int r;
	r = orangefs_writepage_locked(page, wbc);
	unlock_page(page);
	return r;
}

static int orangefs_readpage(struct file *file, struct page *page)
{
	struct inode *inode = page->mapping->host;
	struct iov_iter iter;
	struct bio_vec bv;
	ssize_t ret;
	loff_t off;

	off = page_offset(page);
	trace_orangefs_readpage(off, PAGE_SIZE);
	bv.bv_page = page;
	bv.bv_len = PAGE_SIZE;
	bv.bv_offset = 0;
	iov_iter_bvec(&iter, ITER_BVEC | READ, &bv, 1, PAGE_SIZE);

	ret = wait_for_direct_io(ORANGEFS_IO_READ, inode, &off, &iter,
	    PAGE_SIZE, inode->i_size, NULL);
	/* this will only zero remaining unread portions of the page data */
	iov_iter_zero(~0U, &iter);
	/* takes care of potential aliasing */
	flush_dcache_page(page);
	if (ret < 0) {
		SetPageError(page);
	} else {
		SetPageUptodate(page);
		if (PageError(page))
			ClearPageError(page);
		ret = 0;
	}
	/* unlock the page after the ->readpage() routine completes */
	unlock_page(page);
	return ret;
}

static int orangefs_write_begin(struct file *file,
    struct address_space *mapping, loff_t pos, unsigned len, unsigned flags,
    struct page **pagep, void **fsdata)
{
	int r;
	r = simple_write_begin(file, mapping, pos, len, flags, pagep, fsdata);
	if (r)
		return r;
	r = do_writepage_if_necessary(*pagep, pos, len);
	if (r)
		unlock_page(*pagep);
	return r;
}

int orangefs_write_end(struct file *file, struct address_space *mapping,
    loff_t pos, unsigned len, unsigned copied, struct page *page, void *fsdata)
{
	int r;
	if (update_wr(page, pos, len, 0))
		return -ENOMEM;
	r = simple_write_end(file, mapping, pos, len, copied, page, fsdata);
	mark_inode_dirty_sync(file_inode(file));
	return r;
}

static void orangefs_invalidatepage(struct page *page,
				 unsigned int offset,
				 unsigned int length)
{
	struct orangefs_write_request *wr;
	/* XXX move to releasepage and call + rebase */
	struct writeback_control wbc = {
		.sync_mode = WB_SYNC_ALL,
		.nr_to_write = 0,
	};
	int r;
	if (PagePrivate(page)) {
		wr = (struct orangefs_write_request *)page_private(page);
		BUG_ON(!wr);
/* XXX prove */
		if (offset == 0 && length == PAGE_SIZE) {
			ClearPagePrivate(page);
			kfree(wr);
		} else if (wr->pos - page_offset(page) < offset &&
		    wr->pos - page_offset(page) + wr->len > offset + length) {
			wbc.range_start = page_file_offset(page);
			wbc.range_end = wbc.range_start + PAGE_SIZE - 1;
			wait_on_page_writeback(page);
			if (clear_page_dirty_for_io(page)) {
				trace_orangefs_early_writeback(0);
				r = orangefs_writepage_locked(page, &wbc);
				if (r)
					return;
			} else {
				ClearPagePrivate(page);
				kfree(wr);
			}
		} else if (wr->pos - page_offset(page) < offset &&
		    wr->pos - page_offset(page) + wr->len <= offset + length) {
			wr->len = offset;
		} else if (wr->pos - page_offset(page) >= offset &&
		    wr->pos - page_offset(page) + wr->len > offset + length) {
			wr->pos += length - wr->pos + page_offset(page);
			wr->len -= length - wr->pos + page_offset(page);
		} else {
			/*
			 * Invalidate range is bigger than write range but
			 * entire write range is to be invalidated.
			 */
			ClearPagePrivate(page);
			kfree(wr);
		}
	}
	return;

}

static int orangefs_releasepage(struct page *page, gfp_t foo)
{
	BUG();
	return !PagePrivate(page);
}

static ssize_t orangefs_direct_IO(struct kiocb *iocb,
				  struct iov_iter *iter)
{
	struct file *file = iocb->ki_filp;
	loff_t pos = *(&iocb->ki_pos);
	return do_readv_writev(iov_iter_rw(iter) == WRITE ?
	    ORANGEFS_IO_WRITE : ORANGEFS_IO_READ, file, &pos, iter);
}

/** ORANGEFS2 implementation of address space operations */
static const struct address_space_operations orangefs_address_operations = {
	.writepage = orangefs_writepage,
	.readpage = orangefs_readpage,
	.set_page_dirty = __set_page_dirty_nobuffers,
	.write_begin = orangefs_write_begin,
	.write_end = orangefs_write_end,
	.invalidatepage = orangefs_invalidatepage,
	.releasepage = orangefs_releasepage,
	.direct_IO = orangefs_direct_IO,
};

static int orangefs_setattr_size(struct inode *inode, struct iattr *iattr)
{
	struct orangefs_inode_s *orangefs_inode = ORANGEFS_I(inode);
	struct orangefs_kernel_op_s *new_op;
	loff_t orig_size;
	int ret = -EINVAL;

	gossip_debug(GOSSIP_INODE_DEBUG,
		     "%s: %pU: Handle is %pU | fs_id %d | size is %llu\n",
		     __func__,
		     get_khandle_from_ino(inode),
		     &orangefs_inode->refn.khandle,
		     orangefs_inode->refn.fs_id,
		     iattr->ia_size);

	/* Ensure that we have a up to date size, so we know if it changed. */
	ret = orangefs_inode_getattr(inode, ORANGEFS_GETATTR_SIZE);
	if (ret == -ESTALE)
		ret = -EIO;
	if (ret) {
		gossip_err("%s: orangefs_inode_getattr failed, ret:%d:.\n",
		    __func__, ret);
		return ret;
	}
	orig_size = i_size_read(inode);

	truncate_setsize(inode, iattr->ia_size);

	new_op = op_alloc(ORANGEFS_VFS_OP_TRUNCATE);
	if (!new_op)
		return -ENOMEM;

	new_op->upcall.req.truncate.refn = orangefs_inode->refn;
	new_op->upcall.req.truncate.size = (__s64) iattr->ia_size;

	ret = service_operation(new_op,
				get_interruptible_flag(inode));

	/*
	 * the truncate has no downcall members to retrieve, but
	 * the status value tells us if it went through ok or not
	 */
	gossip_debug(GOSSIP_INODE_DEBUG,
		     "orangefs: orangefs_truncate got return value of %d\n",
		     ret);

	op_release(new_op);

	if (ret != 0)
		return ret;

	if (orig_size != i_size_read(inode))
		iattr->ia_valid |= ATTR_CTIME | ATTR_MTIME;

	return ret;
}

int __orangefs_setattr(struct inode *inode, struct iattr *iattr)
{
	int ret;

	if (iattr->ia_valid & ATTR_MODE) {
		if (iattr->ia_mode & (S_ISVTX)) {
			if (is_root_handle(inode)) {
				/*
				 * allow sticky bit to be set on root (since
				 * it shows up that way by default anyhow),
				 * but don't show it to the server
				 */
				iattr->ia_mode -= S_ISVTX;
			} else {
				gossip_debug(GOSSIP_UTILS_DEBUG,
					     "User attempted to set sticky bit on non-root directory; returning EINVAL.\n");
				return -EINVAL;
			}
		}
		if (iattr->ia_mode & (S_ISUID)) {
			gossip_debug(GOSSIP_UTILS_DEBUG,
				     "Attempting to set setuid bit (not supported); returning EINVAL.\n");
			return -EINVAL;
		}
	}

	if (iattr->ia_valid & ATTR_SIZE) {
		ret = orangefs_setattr_size(inode, iattr);
		if (ret)
			goto out;
	}

again:
	spin_lock(&inode->i_lock);
	if (ORANGEFS_I(inode)->attr_valid) {
		if (uid_eq(ORANGEFS_I(inode)->attr_uid, current_fsuid()) &&
		    gid_eq(ORANGEFS_I(inode)->attr_gid, current_fsgid())) {
			ORANGEFS_I(inode)->attr_valid = iattr->ia_valid;
		} else {
			spin_unlock(&inode->i_lock);
			write_inode_now(inode, 1);
			goto again;
		}
	} else {
		ORANGEFS_I(inode)->attr_valid = iattr->ia_valid;
		ORANGEFS_I(inode)->attr_uid = current_fsuid();
		ORANGEFS_I(inode)->attr_gid = current_fsgid();
	}
	setattr_copy(inode, iattr);
	spin_unlock(&inode->i_lock);
	mark_inode_dirty(inode);

	if (iattr->ia_valid & ATTR_MODE)
		/* change mod on a file that has ACLs */
		ret = posix_acl_chmod(inode, inode->i_mode);

	ret = 0;
out:
	return ret;

}

/*
 * Change attributes of an object referenced by dentry.
 */
int orangefs_setattr(struct dentry *dentry, struct iattr *iattr)
{
	int ret;
	gossip_debug(GOSSIP_INODE_DEBUG, "__orangefs_setattr: called on %pd\n",
	    dentry);
	ret = setattr_prepare(dentry, iattr);
	if (ret)
		goto out;
	ret = __orangefs_setattr(d_inode(dentry), iattr);
	sync_inode_metadata(d_inode(dentry), 1);
out:
	gossip_debug(GOSSIP_INODE_DEBUG, "orangefs_setattr: returning %d\n",
	    ret);
	return ret;
}

/*
 * Obtain attributes of an object given a dentry
 */
int orangefs_getattr(const struct path *path, struct kstat *stat,
		     u32 request_mask, unsigned int flags)
{
	int ret = -ENOENT;
	struct inode *inode = path->dentry->d_inode;
	struct orangefs_inode_s *orangefs_inode = NULL;

	gossip_debug(GOSSIP_INODE_DEBUG,
		     "orangefs_getattr: called on %pd mask %u\n",
		     path->dentry, request_mask);

	ret = orangefs_inode_getattr(inode,
	    request_mask & STATX_SIZE ? ORANGEFS_GETATTR_SIZE : 0);
	if (ret == 0) {
		generic_fillattr(inode, stat);

		/* override block size reported to stat */
		orangefs_inode = ORANGEFS_I(inode);
		stat->blksize = orangefs_inode->blksize;

		if (request_mask & STATX_SIZE)
			stat->result_mask = STATX_BASIC_STATS;
		else
			stat->result_mask = STATX_BASIC_STATS &
			    ~STATX_SIZE;
	}
	return ret;
}

int orangefs_permission(struct inode *inode, int mask)
{
	int ret;

	if (mask & MAY_NOT_BLOCK)
		return -ECHILD;

	gossip_debug(GOSSIP_INODE_DEBUG, "%s: refreshing\n", __func__);

	/* Make sure the permission (and other common attrs) are up to date. */
	ret = orangefs_inode_getattr(inode, 0);
	if (ret < 0)
		return ret;

	return generic_permission(inode, mask);
}

int orangefs_update_time(struct inode *inode, struct timespec *time, int flags)
{
	struct iattr iattr;
	gossip_debug(GOSSIP_INODE_DEBUG, "orangefs_update_time: %pU\n",
	    get_khandle_from_ino(inode));
	generic_update_time(inode, time, flags);
	memset(&iattr, 0, sizeof iattr);
        if (flags & S_ATIME)
		iattr.ia_valid |= ATTR_ATIME;
	if (flags & S_CTIME)
		iattr.ia_valid |= ATTR_CTIME;
	if (flags & S_MTIME)
		iattr.ia_valid |= ATTR_MTIME;
	return __orangefs_setattr(inode, &iattr);
}

/* ORANGEDS2 implementation of VFS inode operations for files */
static const struct inode_operations orangefs_file_inode_operations = {
	.get_acl = orangefs_get_acl,
	.set_acl = orangefs_set_acl,
	.setattr = orangefs_setattr,
	.getattr = orangefs_getattr,
	.listxattr = orangefs_listxattr,
	.permission = orangefs_permission,
	.update_time = orangefs_update_time,
};

static int orangefs_init_iops(struct inode *inode)
{
	inode->i_mapping->a_ops = &orangefs_address_operations;

	switch (inode->i_mode & S_IFMT) {
	case S_IFREG:
		inode->i_op = &orangefs_file_inode_operations;
		inode->i_fop = &orangefs_file_operations;
		inode->i_blkbits = PAGE_SHIFT;
		break;
	case S_IFLNK:
		inode->i_op = &orangefs_symlink_inode_operations;
		break;
	case S_IFDIR:
		inode->i_op = &orangefs_dir_inode_operations;
		inode->i_fop = &orangefs_dir_operations;
		break;
	default:
		gossip_debug(GOSSIP_INODE_DEBUG,
			     "%s: unsupported mode\n",
			     __func__);
		return -EINVAL;
	}

	return 0;
}

/*
 * Given a ORANGEFS object identifier (fsid, handle), convert it into a ino_t type
 * that will be used as a hash-index from where the handle will
 * be searched for in the VFS hash table of inodes.
 */
static inline ino_t orangefs_handle_hash(struct orangefs_object_kref *ref)
{
	if (!ref)
		return 0;
	return orangefs_khandle_to_ino(&(ref->khandle));
}

/*
 * Called to set up an inode from iget5_locked.
 */
static int orangefs_set_inode(struct inode *inode, void *data)
{
	struct orangefs_object_kref *ref = (struct orangefs_object_kref *) data;
	ORANGEFS_I(inode)->refn.fs_id = ref->fs_id;
	ORANGEFS_I(inode)->refn.khandle = ref->khandle;
	ORANGEFS_I(inode)->attr_valid = 0;
	hash_init(ORANGEFS_I(inode)->xattr_cache);
	return 0;
}

/*
 * Called to determine if handles match.
 */
static int orangefs_test_inode(struct inode *inode, void *data)
{
	struct orangefs_object_kref *ref = (struct orangefs_object_kref *) data;
	struct orangefs_inode_s *orangefs_inode = NULL;

	orangefs_inode = ORANGEFS_I(inode);
	return (!ORANGEFS_khandle_cmp(&(orangefs_inode->refn.khandle), &(ref->khandle))
		&& orangefs_inode->refn.fs_id == ref->fs_id);
}

/*
 * Front-end to lookup the inode-cache maintained by the VFS using the ORANGEFS
 * file handle.
 *
 * @sb: the file system super block instance.
 * @ref: The ORANGEFS object for which we are trying to locate an inode structure.
 */
struct inode *orangefs_iget(struct super_block *sb, struct orangefs_object_kref *ref)
{
	struct inode *inode = NULL;
	unsigned long hash;
	int error;

	hash = orangefs_handle_hash(ref);
	inode = iget5_locked(sb, hash, orangefs_test_inode, orangefs_set_inode, ref);
	if (!inode || !(inode->i_state & I_NEW))
		return inode;

	error = orangefs_inode_getattr(inode, ORANGEFS_GETATTR_NEW);
	if (error) {
		iget_failed(inode);
		return ERR_PTR(error);
	}

	inode->i_ino = hash;	/* needed for stat etc */
	orangefs_init_iops(inode);
	unlock_new_inode(inode);

	gossip_debug(GOSSIP_INODE_DEBUG,
		     "iget handle %pU, fsid %d hash %ld i_ino %lu\n",
		     &ref->khandle,
		     ref->fs_id,
		     hash,
		     inode->i_ino);

	return inode;
}

/*
 * Allocate an inode for a newly created file and insert it into the inode hash.
 */
struct inode *orangefs_new_inode(struct super_block *sb, struct inode *dir,
		int mode, dev_t dev, struct orangefs_object_kref *ref)
{
	unsigned long hash = orangefs_handle_hash(ref);
	struct inode *inode;
	int error;

	gossip_debug(GOSSIP_INODE_DEBUG,
		     "%s:(sb is %p | MAJOR(dev)=%u | MINOR(dev)=%u mode=%o)\n",
		     __func__,
		     sb,
		     MAJOR(dev),
		     MINOR(dev),
		     mode);

	inode = new_inode(sb);
	if (!inode)
		return NULL;

	orangefs_set_inode(inode, ref);
	inode->i_ino = hash;	/* needed for stat etc */

	error = orangefs_inode_getattr(inode, ORANGEFS_GETATTR_NEW);
	if (error)
		goto out_iput;

	orangefs_init_iops(inode);
	inode->i_rdev = dev;

	error = insert_inode_locked4(inode, hash, orangefs_test_inode, ref);
	if (error < 0)
		goto out_iput;

	gossip_debug(GOSSIP_INODE_DEBUG,
		     "Initializing ACL's for inode %pU\n",
		     get_khandle_from_ino(inode));
	orangefs_init_acl(inode, dir);
	return inode;

out_iput:
	iput(inode);
	return ERR_PTR(error);
}
