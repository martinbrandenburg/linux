/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) 2001 Clemson University and The University of Chicago
 * Copyright 2018 Omnibond Systems, L.L.C.
 *
 * See COPYING in top-level directory.
 */

#ifndef _UAPI_LINUX_ORANGEFS_H
#define _UAPI_LINUX_ORANGEFS_H

#include <linux/ioctl.h>
#include <linux/types.h>

/*
 * valid orangefs kernel operation types
 */
#define ORANGEFS_VFS_OP_INVALID      0xFF000000
#define ORANGEFS_VFS_OP_FILE_IO      0xFF000001
#define ORANGEFS_VFS_OP_LOOKUP       0xFF000002
#define ORANGEFS_VFS_OP_CREATE       0xFF000003
#define ORANGEFS_VFS_OP_GETATTR      0xFF000004
#define ORANGEFS_VFS_OP_REMOVE       0xFF000005
#define ORANGEFS_VFS_OP_MKDIR        0xFF000006
#define ORANGEFS_VFS_OP_READDIR      0xFF000007
#define ORANGEFS_VFS_OP_SETATTR      0xFF000008
#define ORANGEFS_VFS_OP_SYMLINK      0xFF000009
#define ORANGEFS_VFS_OP_RENAME       0xFF00000A
#define ORANGEFS_VFS_OP_STATFS       0xFF00000B
#define ORANGEFS_VFS_OP_TRUNCATE     0xFF00000C
#define ORANGEFS_VFS_OP_RA_FLUSH     0xFF00000D
#define ORANGEFS_VFS_OP_FS_MOUNT     0xFF00000E
#define ORANGEFS_VFS_OP_FS_UMOUNT    0xFF00000F
#define ORANGEFS_VFS_OP_GETXATTR     0xFF000010
#define ORANGEFS_VFS_OP_SETXATTR     0xFF000011
#define ORANGEFS_VFS_OP_LISTXATTR    0xFF000012
#define ORANGEFS_VFS_OP_REMOVEXATTR  0xFF000013
#define ORANGEFS_VFS_OP_PARAM        0xFF000014
#define ORANGEFS_VFS_OP_PERF_COUNT   0xFF000015
#define ORANGEFS_VFS_OP_CANCEL       0xFF00EE00
#define ORANGEFS_VFS_OP_FSYNC        0xFF00EE01
#define ORANGEFS_VFS_OP_FSKEY        0xFF00EE02
#define ORANGEFS_VFS_OP_READDIRPLUS  0xFF00EE03
#define ORANGEFS_VFS_OP_FEATURES     0xFF00EE05 /* 2.9.6 */

/* features is a 64-bit unsigned bitmask */
#define ORANGEFS_FEATURE_READAHEAD 1

/*
 * Misc constants. Please retain them as multiples of 8!
 * Otherwise 32-64 bit interactions will be messed up :)
 */
#define ORANGEFS_MAX_DEBUG_STRING_LEN 0x00000800

#define ORANGEFS_MAX_DIRENT_COUNT_READDIR 512

/*
 * The 2.9 core will put 64 bit handles in here like this:
 *    1234 0000 0000 5678
 * The 3.0 and beyond cores will put 128 bit handles in here like this:
 *    1234 5678 90AB CDEF
 * The kernel module will always use the first four bytes and
 * the last four bytes as an inum.
 */
struct orangefs_khandle {
	unsigned char u[16];
} __attribute__((aligned(8)));

/*
 * kernel version of an object ref.
 */
struct orangefs_object_kref {
	struct orangefs_khandle khandle;
	__s32 fs_id;
	__s32 __pad1;
};

/*
 * ORANGEFS error codes are a signed 32-bit integer. Error codes are negative, but
 * the sign is stripped before decoding.
 */

/* Bit 31 is not used since it is the sign. */

/*
 * Bit 30 specifies that this is a ORANGEFS error. A ORANGEFS error is either an
 * encoded errno value or a ORANGEFS protocol error.
 */
#define ORANGEFS_ERROR_BIT (1 << 30)

/*
 * Bit 29 specifies that this is a ORANGEFS protocol error and not an encoded
 * errno value.
 */
#define ORANGEFS_NON_ERRNO_ERROR_BIT (1 << 29)

/*
 * Bits 9, 8, and 7 specify the error class, which encodes the section of
 * server code the error originated in for logging purposes. It is not used
 * in the kernel except to be masked out.
 */
#define ORANGEFS_ERROR_CLASS_BITS 0x380

/* Bits 6 - 0 are reserved for the actual error code. */
#define ORANGEFS_ERROR_NUMBER_BITS 0x7f

/* Encoded errno values decoded by PINT_errno_mapping in orangefs-utils.c. */

/* Our own ORANGEFS protocol error codes. */
#define ORANGEFS_ECANCEL    (1|ORANGEFS_NON_ERRNO_ERROR_BIT|ORANGEFS_ERROR_BIT)
#define ORANGEFS_EDEVINIT   (2|ORANGEFS_NON_ERRNO_ERROR_BIT|ORANGEFS_ERROR_BIT)
#define ORANGEFS_EDETAIL    (3|ORANGEFS_NON_ERRNO_ERROR_BIT|ORANGEFS_ERROR_BIT)
#define ORANGEFS_EHOSTNTFD  (4|ORANGEFS_NON_ERRNO_ERROR_BIT|ORANGEFS_ERROR_BIT)
#define ORANGEFS_EADDRNTFD  (5|ORANGEFS_NON_ERRNO_ERROR_BIT|ORANGEFS_ERROR_BIT)
#define ORANGEFS_ENORECVR   (6|ORANGEFS_NON_ERRNO_ERROR_BIT|ORANGEFS_ERROR_BIT)
#define ORANGEFS_ETRYAGAIN  (7|ORANGEFS_NON_ERRNO_ERROR_BIT|ORANGEFS_ERROR_BIT)
#define ORANGEFS_ENOTPVFS   (8|ORANGEFS_NON_ERRNO_ERROR_BIT|ORANGEFS_ERROR_BIT)
#define ORANGEFS_ESECURITY  (9|ORANGEFS_NON_ERRNO_ERROR_BIT|ORANGEFS_ERROR_BIT)

/* permission bits */
#define ORANGEFS_O_EXECUTE (1 << 0)
#define ORANGEFS_O_WRITE   (1 << 1)
#define ORANGEFS_O_READ    (1 << 2)
#define ORANGEFS_G_EXECUTE (1 << 3)
#define ORANGEFS_G_WRITE   (1 << 4)
#define ORANGEFS_G_READ    (1 << 5)
#define ORANGEFS_U_EXECUTE (1 << 6)
#define ORANGEFS_U_WRITE   (1 << 7)
#define ORANGEFS_U_READ    (1 << 8)
/* no ORANGEFS_U_VTX (sticky bit) */
#define ORANGEFS_G_SGID    (1 << 10)
#define ORANGEFS_U_SUID    (1 << 11)

#define ORANGEFS_ITERATE_START  2147483646
#define ORANGEFS_ITERATE_END    2147483645
#define ORANGEFS_IMMUTABLE_FL   FS_IMMUTABLE_FL
#define ORANGEFS_APPEND_FL      FS_APPEND_FL
#define ORANGEFS_NOATIME_FL     FS_NOATIME_FL
#define ORANGEFS_MIRROR_FL      0x01000000ULL
#define ORANGEFS_FS_ID_NULL     ((__s32)0)
#define ORANGEFS_ATTR_SYS_UID                  (1 << 0)
#define ORANGEFS_ATTR_SYS_GID                  (1 << 1)
#define ORANGEFS_ATTR_SYS_PERM                 (1 << 2)
#define ORANGEFS_ATTR_SYS_ATIME                (1 << 3)
#define ORANGEFS_ATTR_SYS_CTIME                (1 << 4)
#define ORANGEFS_ATTR_SYS_MTIME                (1 << 5)
#define ORANGEFS_ATTR_SYS_TYPE                 (1 << 6)
#define ORANGEFS_ATTR_SYS_ATIME_SET            (1 << 7)
#define ORANGEFS_ATTR_SYS_MTIME_SET            (1 << 8)
#define ORANGEFS_ATTR_SYS_SIZE                 (1 << 20)
#define ORANGEFS_ATTR_SYS_LNK_TARGET           (1 << 24)
#define ORANGEFS_ATTR_SYS_DFILE_COUNT          (1 << 25)
#define ORANGEFS_ATTR_SYS_DIRENT_COUNT         (1 << 26)
#define ORANGEFS_ATTR_SYS_BLKSIZE              (1 << 28)
#define ORANGEFS_ATTR_SYS_MIRROR_COPIES_COUNT  (1 << 29)
#define ORANGEFS_ATTR_SYS_COMMON_ALL	\
	(ORANGEFS_ATTR_SYS_UID	|	\
	 ORANGEFS_ATTR_SYS_GID	|	\
	 ORANGEFS_ATTR_SYS_PERM	|	\
	 ORANGEFS_ATTR_SYS_ATIME	|	\
	 ORANGEFS_ATTR_SYS_CTIME	|	\
	 ORANGEFS_ATTR_SYS_MTIME	|	\
	 ORANGEFS_ATTR_SYS_TYPE)

#define ORANGEFS_ATTR_SYS_ALL_SETABLE		\
(ORANGEFS_ATTR_SYS_COMMON_ALL-ORANGEFS_ATTR_SYS_TYPE)

#define ORANGEFS_ATTR_SYS_ALL_NOHINT			\
	(ORANGEFS_ATTR_SYS_COMMON_ALL		|	\
	 ORANGEFS_ATTR_SYS_SIZE			|	\
	 ORANGEFS_ATTR_SYS_LNK_TARGET		|	\
	 ORANGEFS_ATTR_SYS_DFILE_COUNT		|	\
	 ORANGEFS_ATTR_SYS_MIRROR_COPIES_COUNT	|	\
	 ORANGEFS_ATTR_SYS_DIRENT_COUNT		|	\
	 ORANGEFS_ATTR_SYS_BLKSIZE)

#define ORANGEFS_XATTR_REPLACE 0x2
#define ORANGEFS_XATTR_CREATE  0x1
#define ORANGEFS_MAX_SERVER_ADDR_LEN 256
#define ORANGEFS_NAME_MAX                256
/*
 * max extended attribute name len as imposed by the VFS and exploited for the
 * upcall request types.
 * NOTE: Please retain them as multiples of 8 even if you wish to change them
 * This is *NECESSARY* for supporting 32 bit user-space binaries on a 64-bit
 * kernel. Due to implementation within DBPF, this really needs to be
 * ORANGEFS_NAME_MAX, which it was the same value as, but no reason to let it
 * break if that changes in the future.
 */
#define ORANGEFS_MAX_XATTR_NAMELEN   ORANGEFS_NAME_MAX	/* Not the same as
						 * XATTR_NAME_MAX defined
						 * by <linux/xattr.h>
						 */
#define ORANGEFS_MAX_XATTR_VALUELEN  8192	/* Not the same as XATTR_SIZE_MAX
					 * defined by <linux/xattr.h>
					 */
#define ORANGEFS_MAX_XATTR_LISTLEN   16	/* Not the same as XATTR_LIST_MAX
					 * defined by <linux/xattr.h>
					 */
/*
 * ORANGEFS I/O operation types, used in both system and server interfaces.
 */
enum ORANGEFS_io_type {
	ORANGEFS_IO_READ = 1,
	ORANGEFS_IO_WRITE = 2
};

/*
 * If this enum is modified the server parameters related to the precreate pool
 * batch and low threshold sizes may need to be modified  to reflect this
 * change.
 */
enum orangefs_ds_type {
	ORANGEFS_TYPE_NONE = 0,
	ORANGEFS_TYPE_METAFILE = (1 << 0),
	ORANGEFS_TYPE_DATAFILE = (1 << 1),
	ORANGEFS_TYPE_DIRECTORY = (1 << 2),
	ORANGEFS_TYPE_SYMLINK = (1 << 3),
	ORANGEFS_TYPE_DIRDATA = (1 << 4),
	ORANGEFS_TYPE_INTERNAL = (1 << 5)	/* for the server's private use */
};

/* This structure is used by the VFS-client interaction alone */
struct ORANGEFS_keyval_pair {
	char key[ORANGEFS_MAX_XATTR_NAMELEN];
	__s32 key_sz;	/* __s32 for portable, fixed-size structures */
	__s32 val_sz;
	char val[ORANGEFS_MAX_XATTR_VALUELEN];
};

/* pvfs2-sysint.h ***********************************************************/
/* Describes attributes for a file, directory, or symlink. */
struct ORANGEFS_sys_attr_s {
	__u32 owner;
	__u32 group;
	__u32 perms;
	__u64 atime;
	__u64 mtime;
	__u64 ctime;
	__s64 size;

	/* NOTE: caller must free if valid */
	char *link_target;

	/* Changed to __s32 so that size of structure does not change */
	__s32 dfile_count;

	/* Changed to __s32 so that size of structure does not change */
	__s32 distr_dir_servers_initial;

	/* Changed to __s32 so that size of structure does not change */
	__s32 distr_dir_servers_max;

	/* Changed to __s32 so that size of structure does not change */
	__s32 distr_dir_split_size;

	__u32 mirror_copies_count;

	/* NOTE: caller must free if valid */
	char *dist_name;

	/* NOTE: caller must free if valid */
	char *dist_params;

	__s64 dirent_count;
	enum orangefs_ds_type objtype;
	__u64 flags;
	__u32 mask;
	__s64 blksize;
};

#define ORANGEFS_LOOKUP_LINK_NO_FOLLOW 0

/* pint-dev.h ***************************************************************/

/* parameter structure used in ORANGEFS_DEV_DEBUG ioctl command */
struct dev_mask_info_s {
	enum {
		KERNEL_MASK,
		CLIENT_MASK,
	} mask_type;
	__u64 mask_value;
};

struct dev_mask2_info_s {
	__u64 mask1_value;
	__u64 mask2_value;
};

#define	GOSSIP_NO_DEBUG			(__u64)0

#define GOSSIP_SUPER_DEBUG		((__u64)1 << 0)
#define GOSSIP_INODE_DEBUG		((__u64)1 << 1)
#define GOSSIP_FILE_DEBUG		((__u64)1 << 2)
#define GOSSIP_DIR_DEBUG		((__u64)1 << 3)
#define GOSSIP_UTILS_DEBUG		((__u64)1 << 4)
#define GOSSIP_WAIT_DEBUG		((__u64)1 << 5)
#define GOSSIP_ACL_DEBUG		((__u64)1 << 6)
#define GOSSIP_DCACHE_DEBUG		((__u64)1 << 7)
#define GOSSIP_DEV_DEBUG		((__u64)1 << 8)
#define GOSSIP_NAME_DEBUG		((__u64)1 << 9)
#define GOSSIP_BUFMAP_DEBUG		((__u64)1 << 10)
#define GOSSIP_CACHE_DEBUG		((__u64)1 << 11)
#define GOSSIP_DEBUGFS_DEBUG		((__u64)1 << 12)
#define GOSSIP_XATTR_DEBUG		((__u64)1 << 13)
#define GOSSIP_INIT_DEBUG		((__u64)1 << 14)
#define GOSSIP_SYSFS_DEBUG		((__u64)1 << 15)

#define GOSSIP_MAX_NR                 16
#define GOSSIP_MAX_DEBUG              (((__u64)1 << GOSSIP_MAX_NR) - 1)

/* pint-dev-shared.h ********************************************************/
#define ORANGEFS_DEV_MAGIC 'k'

#define ORANGEFS_READDIR_DEFAULT_DESC_COUNT  5

#define DEV_GET_MAGIC           0x1
#define DEV_GET_MAX_UPSIZE      0x2
#define DEV_GET_MAX_DOWNSIZE    0x3
#define DEV_MAP                 0x4
#define DEV_REMOUNT_ALL         0x5
#define DEV_DEBUG               0x6
#define DEV_UPSTREAM            0x7
#define DEV_CLIENT_MASK         0x8
#define DEV_CLIENT_STRING       0x9
#define DEV_MAX_NR              0xa

/* supported ioctls, codes are with respect to user-space */
enum {
	ORANGEFS_DEV_GET_MAGIC = _IOW(ORANGEFS_DEV_MAGIC, DEV_GET_MAGIC, __s32),
	ORANGEFS_DEV_GET_MAX_UPSIZE =
	    _IOW(ORANGEFS_DEV_MAGIC, DEV_GET_MAX_UPSIZE, __s32),
	ORANGEFS_DEV_GET_MAX_DOWNSIZE =
	    _IOW(ORANGEFS_DEV_MAGIC, DEV_GET_MAX_DOWNSIZE, __s32),
	ORANGEFS_DEV_MAP = _IO(ORANGEFS_DEV_MAGIC, DEV_MAP),
	ORANGEFS_DEV_REMOUNT_ALL = _IO(ORANGEFS_DEV_MAGIC, DEV_REMOUNT_ALL),
	ORANGEFS_DEV_DEBUG = _IOR(ORANGEFS_DEV_MAGIC, DEV_DEBUG, __s32),
	ORANGEFS_DEV_UPSTREAM = _IOW(ORANGEFS_DEV_MAGIC, DEV_UPSTREAM, int),
	ORANGEFS_DEV_CLIENT_MASK = _IOW(ORANGEFS_DEV_MAGIC,
				    DEV_CLIENT_MASK,
				    struct dev_mask2_info_s),
	ORANGEFS_DEV_CLIENT_STRING = _IOW(ORANGEFS_DEV_MAGIC,
				      DEV_CLIENT_STRING,
				      char *),
	ORANGEFS_DEV_MAXNR = DEV_MAX_NR,
};

/*
 * version number for use in communicating between kernel space and user
 * space. Zero signifies the upstream version of the kernel module.
 */
#define ORANGEFS_KERNEL_PROTO_VERSION 0
#define ORANGEFS_MINIMUM_USERSPACE_VERSION 20903

/*
 * describes memory regions to map in the ORANGEFS_DEV_MAP ioctl.
 * NOTE: See devorangefs-req.c for 32 bit compat structure.
 * Since this structure has a variable-sized layout that is different
 * on 32 and 64 bit platforms, we need to normalize to a 64 bit layout
 * on such systems before servicing ioctl calls from user-space binaries
 * that may be 32 bit!
 */
struct ORANGEFS_dev_map_desc {
	void *ptr;
	__s32 total_size;
	__s32 size;
	__s32 count;
};

struct orangefs_io_response {
	__s64 amt_complete;
};

struct orangefs_lookup_response {
	struct orangefs_object_kref refn;
};

struct orangefs_create_response {
	struct orangefs_object_kref refn;
};

struct orangefs_symlink_response {
	struct orangefs_object_kref refn;
};

struct orangefs_getattr_response {
	struct ORANGEFS_sys_attr_s attributes;
	char link_target[ORANGEFS_NAME_MAX];
};

struct orangefs_mkdir_response {
	struct orangefs_object_kref refn;
};

struct orangefs_statfs_response {
	__s64 block_size;
	__s64 blocks_total;
	__s64 blocks_avail;
	__s64 files_total;
	__s64 files_avail;
};

struct orangefs_fs_mount_response {
	__s32 fs_id;
	__s32 id;
	struct orangefs_khandle root_khandle;
};

/* the getxattr response is the attribute value */
struct orangefs_getxattr_response {
	__s32 val_sz;
	__s32 __pad1;
	char val[ORANGEFS_MAX_XATTR_VALUELEN];
};

/* the listxattr response is an array of attribute names */
struct orangefs_listxattr_response {
	__s32 returned_count;
	__s32 __pad1;
	__u64 token;
	char key[ORANGEFS_MAX_XATTR_LISTLEN * ORANGEFS_MAX_XATTR_NAMELEN];
	__s32 keylen;
	__s32 __pad2;
	__s32 lengths[ORANGEFS_MAX_XATTR_LISTLEN];
};

struct orangefs_param_response {
	union {
		__s64 value64;
		__s32 value32[2];
	} u;
};

#define PERF_COUNT_BUF_SIZE 4096
struct orangefs_perf_count_response {
	char buffer[PERF_COUNT_BUF_SIZE];
};

#define FS_KEY_BUF_SIZE 4096
struct orangefs_fs_key_response {
	__s32 fs_keylen;
	__s32 __pad1;
	char fs_key[FS_KEY_BUF_SIZE];
};

/* 2.9.6 */
struct orangefs_features_response {
	__u64 features;
};

struct orangefs_downcall_s {
	__s32 type;
	__s32 status;
	/* currently trailer is used only by readdir */
	__s64 trailer_size;
	char *trailer_buf;

	union {
		struct orangefs_io_response io;
		struct orangefs_lookup_response lookup;
		struct orangefs_create_response create;
		struct orangefs_symlink_response sym;
		struct orangefs_getattr_response getattr;
		struct orangefs_mkdir_response mkdir;
		struct orangefs_statfs_response statfs;
		struct orangefs_fs_mount_response fs_mount;
		struct orangefs_getxattr_response getxattr;
		struct orangefs_listxattr_response listxattr;
		struct orangefs_param_response param;
		struct orangefs_perf_count_response perf_count;
		struct orangefs_fs_key_response fs_key;
		struct orangefs_features_response features;
	} resp;
};

/*
 * The readdir response comes in the trailer.  It is followed by the
 * directory entries as described in dir.c.
 */

struct orangefs_readdir_response_s {
	__u64 token;
	__u64 directory_version;
	__u32 __pad2;
	__u32 orangefs_dirent_outcount;
};

struct orangefs_io_request_s {
	__s32 __pad1;
	__s32 buf_index;
	__s32 count;
	__s32 __pad2;
	__s64 offset;
	struct orangefs_object_kref refn;
	enum ORANGEFS_io_type io_type;
	__s32 readahead_size;
};

struct orangefs_lookup_request_s {
	__s32 sym_follow;
	__s32 __pad1;
	struct orangefs_object_kref parent_refn;
	char d_name[ORANGEFS_NAME_MAX];
};

struct orangefs_create_request_s {
	struct orangefs_object_kref parent_refn;
	struct ORANGEFS_sys_attr_s attributes;
	char d_name[ORANGEFS_NAME_MAX];
};

struct orangefs_symlink_request_s {
	struct orangefs_object_kref parent_refn;
	struct ORANGEFS_sys_attr_s attributes;
	char entry_name[ORANGEFS_NAME_MAX];
	char target[ORANGEFS_NAME_MAX];
};

struct orangefs_getattr_request_s {
	struct orangefs_object_kref refn;
	__u32 mask;
	__u32 __pad1;
};

struct orangefs_setattr_request_s {
	struct orangefs_object_kref refn;
	struct ORANGEFS_sys_attr_s attributes;
};

struct orangefs_remove_request_s {
	struct orangefs_object_kref parent_refn;
	char d_name[ORANGEFS_NAME_MAX];
};

struct orangefs_mkdir_request_s {
	struct orangefs_object_kref parent_refn;
	struct ORANGEFS_sys_attr_s attributes;
	char d_name[ORANGEFS_NAME_MAX];
};

struct orangefs_readdir_request_s {
	struct orangefs_object_kref refn;
	__u64 token;
	__s32 max_dirent_count;
	__s32 buf_index;
};

struct orangefs_readdirplus_request_s {
	struct orangefs_object_kref refn;
	__u64 token;
	__s32 max_dirent_count;
	__u32 mask;
	__s32 buf_index;
	__s32 __pad1;
};

struct orangefs_rename_request_s {
	struct orangefs_object_kref old_parent_refn;
	struct orangefs_object_kref new_parent_refn;
	char d_old_name[ORANGEFS_NAME_MAX];
	char d_new_name[ORANGEFS_NAME_MAX];
};

struct orangefs_statfs_request_s {
	__s32 fs_id;
	__s32 __pad1;
};

struct orangefs_truncate_request_s {
	struct orangefs_object_kref refn;
	__s64 size;
};

struct orangefs_ra_cache_flush_request_s {
	struct orangefs_object_kref refn;
};

struct orangefs_fs_mount_request_s {
	char orangefs_config_server[ORANGEFS_MAX_SERVER_ADDR_LEN];
};

struct orangefs_fs_umount_request_s {
	__s32 id;
	__s32 fs_id;
	char orangefs_config_server[ORANGEFS_MAX_SERVER_ADDR_LEN];
};

struct orangefs_getxattr_request_s {
	struct orangefs_object_kref refn;
	__s32 key_sz;
	__s32 __pad1;
	char key[ORANGEFS_MAX_XATTR_NAMELEN];
};

struct orangefs_setxattr_request_s {
	struct orangefs_object_kref refn;
	struct ORANGEFS_keyval_pair keyval;
	__s32 flags;
	__s32 __pad1;
};

struct orangefs_listxattr_request_s {
	struct orangefs_object_kref refn;
	__s32 requested_count;
	__s32 __pad1;
	__u64 token;
};

struct orangefs_removexattr_request_s {
	struct orangefs_object_kref refn;
	__s32 key_sz;
	__s32 __pad1;
	char key[ORANGEFS_MAX_XATTR_NAMELEN];
};

struct orangefs_op_cancel_s {
	__u64 op_tag;
};

struct orangefs_fsync_request_s {
	struct orangefs_object_kref refn;
};

enum orangefs_param_request_type {
	ORANGEFS_PARAM_REQUEST_SET = 1,
	ORANGEFS_PARAM_REQUEST_GET = 2
};

enum orangefs_param_request_op {
	ORANGEFS_PARAM_REQUEST_OP_ACACHE_TIMEOUT_MSECS = 1,
	ORANGEFS_PARAM_REQUEST_OP_ACACHE_HARD_LIMIT = 2,
	ORANGEFS_PARAM_REQUEST_OP_ACACHE_SOFT_LIMIT = 3,
	ORANGEFS_PARAM_REQUEST_OP_ACACHE_RECLAIM_PERCENTAGE = 4,
	ORANGEFS_PARAM_REQUEST_OP_PERF_TIME_INTERVAL_SECS = 5,
	ORANGEFS_PARAM_REQUEST_OP_PERF_HISTORY_SIZE = 6,
	ORANGEFS_PARAM_REQUEST_OP_PERF_RESET = 7,
	ORANGEFS_PARAM_REQUEST_OP_NCACHE_TIMEOUT_MSECS = 8,
	ORANGEFS_PARAM_REQUEST_OP_NCACHE_HARD_LIMIT = 9,
	ORANGEFS_PARAM_REQUEST_OP_NCACHE_SOFT_LIMIT = 10,
	ORANGEFS_PARAM_REQUEST_OP_NCACHE_RECLAIM_PERCENTAGE = 11,
	ORANGEFS_PARAM_REQUEST_OP_STATIC_ACACHE_TIMEOUT_MSECS = 12,
	ORANGEFS_PARAM_REQUEST_OP_STATIC_ACACHE_HARD_LIMIT = 13,
	ORANGEFS_PARAM_REQUEST_OP_STATIC_ACACHE_SOFT_LIMIT = 14,
	ORANGEFS_PARAM_REQUEST_OP_STATIC_ACACHE_RECLAIM_PERCENTAGE = 15,
	ORANGEFS_PARAM_REQUEST_OP_CLIENT_DEBUG = 16,
	ORANGEFS_PARAM_REQUEST_OP_CCACHE_TIMEOUT_SECS = 17,
	ORANGEFS_PARAM_REQUEST_OP_CCACHE_HARD_LIMIT = 18,
	ORANGEFS_PARAM_REQUEST_OP_CCACHE_SOFT_LIMIT = 19,
	ORANGEFS_PARAM_REQUEST_OP_CCACHE_RECLAIM_PERCENTAGE = 20,
	ORANGEFS_PARAM_REQUEST_OP_CAPCACHE_TIMEOUT_SECS = 21,
	ORANGEFS_PARAM_REQUEST_OP_CAPCACHE_HARD_LIMIT = 22,
	ORANGEFS_PARAM_REQUEST_OP_CAPCACHE_SOFT_LIMIT = 23,
	ORANGEFS_PARAM_REQUEST_OP_CAPCACHE_RECLAIM_PERCENTAGE = 24,
	ORANGEFS_PARAM_REQUEST_OP_TWO_MASK_VALUES = 25,
	ORANGEFS_PARAM_REQUEST_OP_READAHEAD_SIZE = 26,
	ORANGEFS_PARAM_REQUEST_OP_READAHEAD_COUNT = 27,
	ORANGEFS_PARAM_REQUEST_OP_READAHEAD_COUNT_SIZE = 28,
	ORANGEFS_PARAM_REQUEST_OP_READAHEAD_READCNT = 29,
};

struct orangefs_param_request_s {
	enum orangefs_param_request_type type;
	enum orangefs_param_request_op op;
	union {
		__s64 value64;
		__s32 value32[2];
	} u;
	char s_value[ORANGEFS_MAX_DEBUG_STRING_LEN];
};

enum orangefs_perf_count_request_type {
	ORANGEFS_PERF_COUNT_REQUEST_ACACHE = 1,
	ORANGEFS_PERF_COUNT_REQUEST_NCACHE = 2,
	ORANGEFS_PERF_COUNT_REQUEST_CAPCACHE = 3,
};

struct orangefs_perf_count_request_s {
	enum orangefs_perf_count_request_type type;
	__s32 __pad1;
};

struct orangefs_fs_key_request_s {
	__s32 fsid;
	__s32 __pad1;
};

/* 2.9.6 */
struct orangefs_features_request_s {
	__u64 features;
};

struct orangefs_upcall_s {
	__s32 type;
	__u32 uid;
	__u32 gid;
	int pid;
	int tgid;
	/* Trailers unused but must be retained for protocol compatibility. */
	__s64 trailer_size;
	char *trailer_buf;

	union {
		struct orangefs_io_request_s io;
		struct orangefs_lookup_request_s lookup;
		struct orangefs_create_request_s create;
		struct orangefs_symlink_request_s sym;
		struct orangefs_getattr_request_s getattr;
		struct orangefs_setattr_request_s setattr;
		struct orangefs_remove_request_s remove;
		struct orangefs_mkdir_request_s mkdir;
		struct orangefs_readdir_request_s readdir;
		struct orangefs_readdirplus_request_s readdirplus;
		struct orangefs_rename_request_s rename;
		struct orangefs_statfs_request_s statfs;
		struct orangefs_truncate_request_s truncate;
		struct orangefs_ra_cache_flush_request_s ra_cache_flush;
		struct orangefs_fs_mount_request_s fs_mount;
		struct orangefs_fs_umount_request_s fs_umount;
		struct orangefs_getxattr_request_s getxattr;
		struct orangefs_setxattr_request_s setxattr;
		struct orangefs_listxattr_request_s listxattr;
		struct orangefs_removexattr_request_s removexattr;
		struct orangefs_op_cancel_s cancel;
		struct orangefs_fsync_request_s fsync;
		struct orangefs_param_request_s param;
		struct orangefs_perf_count_request_s perf_count;
		struct orangefs_fs_key_request_s fs_key;
		struct orangefs_features_request_s features;
	} req;
};

#endif /* _UAPI_LINUX_ORANGEFS_H */
