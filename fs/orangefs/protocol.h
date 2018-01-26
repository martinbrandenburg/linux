/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <linux/slab.h>
#include <linux/ioctl.h>

/* khandle stuff  ***********************************************************/

/*
 * compare 2 khandles assumes little endian thus from large address to
 * small address
 */
static inline int ORANGEFS_khandle_cmp(const struct orangefs_khandle *kh1,
				   const struct orangefs_khandle *kh2)
{
	int i;

	for (i = 15; i >= 0; i--) {
		if (kh1->u[i] > kh2->u[i])
			return 1;
		if (kh1->u[i] < kh2->u[i])
			return -1;
	}

	return 0;
}

static inline void ORANGEFS_khandle_to(const struct orangefs_khandle *kh,
				   void *p, int size)
{

	memcpy(p, kh->u, 16);
	memset(p + 16, 0, size - 16);

}

static inline void ORANGEFS_khandle_from(struct orangefs_khandle *kh,
				     void *p, int size)
{
	memset(kh, 0, 16);
	memcpy(kh->u, p, 16);

}

/* pvfs2-types.h ************************************************************/

#define ORANGEFS_SUPER_MAGIC 0x20030528

/* pvfs2-util.h *************************************************************/
__s32 ORANGEFS_util_translate_mode(int mode);

/* pvfs2-internal.h *********************************************************/
#define llu(x) (unsigned long long)(x)
#define lld(x) (long long)(x)

/* gossip.h *****************************************************************/

extern __u64 orangefs_gossip_debug_mask;

/* try to avoid function call overhead by checking masks in macro */
#define gossip_debug(mask, fmt, ...)					\
do {									\
	if (orangefs_gossip_debug_mask & (mask))			\
		printk(KERN_DEBUG fmt, ##__VA_ARGS__);			\
} while (0)

#define gossip_err pr_err
