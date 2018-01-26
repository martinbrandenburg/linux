/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) 2001 Clemson University and The University of Chicago
 *
 * See COPYING in top-level directory.
 */

/* This file just defines debugging masks to be used with the gossip
 * logging utility.  All debugging masks for ORANGEFS are kept here to make
 * sure we don't have collisions.
 */

#ifndef __ORANGEFS_DEBUG_H
#define __ORANGEFS_DEBUG_H

#include <linux/types.h>
#include <linux/kernel.h>

/* a private internal type */
struct __keyword_mask_s {
	const char *keyword;
	__u64 mask_val;
};

/*
 * Map all kmod keywords to kmod debug masks here. Keep this
 * structure "packed":
 *
 *   "all" is always last...
 *
 *   keyword     mask_val     index
 *     foo          1           0
 *     bar          2           1
 *     baz          4           2
 *     qux          8           3
 *      .           .           .
 */
static struct __keyword_mask_s s_kmod_keyword_mask_map[] = {
	{"super", GOSSIP_SUPER_DEBUG},
	{"inode", GOSSIP_INODE_DEBUG},
	{"file", GOSSIP_FILE_DEBUG},
	{"dir", GOSSIP_DIR_DEBUG},
	{"utils", GOSSIP_UTILS_DEBUG},
	{"wait", GOSSIP_WAIT_DEBUG},
	{"acl", GOSSIP_ACL_DEBUG},
	{"dcache", GOSSIP_DCACHE_DEBUG},
	{"dev", GOSSIP_DEV_DEBUG},
	{"name", GOSSIP_NAME_DEBUG},
	{"bufmap", GOSSIP_BUFMAP_DEBUG},
	{"cache", GOSSIP_CACHE_DEBUG},
	{"debugfs", GOSSIP_DEBUGFS_DEBUG},
	{"xattr", GOSSIP_XATTR_DEBUG},
	{"init", GOSSIP_INIT_DEBUG},
	{"sysfs", GOSSIP_SYSFS_DEBUG},
	{"none", GOSSIP_NO_DEBUG},
	{"all", GOSSIP_MAX_DEBUG}
};

static const int num_kmod_keyword_mask_map = (int)
	(ARRAY_SIZE(s_kmod_keyword_mask_map));

#endif /* __ORANGEFS_DEBUG_H */
