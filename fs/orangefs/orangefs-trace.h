/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2018 Omnibond Systems, L.L.C.
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM orangefs

#if !defined(_TRACE_ORANGEFS_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_ORANGEFS_H

#include <linux/tracepoint.h>

#define OP_NAME_LEN 64

TRACE_EVENT(orangefs_devreq_poll,
    TP_PROTO(int empty),
    TP_ARGS(empty),
    TP_STRUCT__entry(
        __field(int, empty)
    ),
    TP_fast_assign(
        __entry->empty = empty;
    ),
    TP_printk(
        "empty=%d", __entry->empty
    )
);

TRACE_EVENT(orangefs_devreq_read,
    TP_PROTO(int success, int empty, struct orangefs_kernel_op_s *op),
    TP_ARGS(success, empty, op),
    TP_STRUCT__entry(
        __field(int, success)
        __field(int, empty)
        __array(char, op_name, OP_NAME_LEN)
    ),
    TP_fast_assign(
        __entry->success = success;
        __entry->empty = empty;
        if (op)
            strlcpy(__entry->op_name, get_opname_string(op), OP_NAME_LEN);
        else
            __entry->op_name[0] = 0;
    ),
    TP_printk(
        "success=%d empty=%d op_name=%s", __entry->success, __entry->empty,
        __entry->op_name
    )
);

TRACE_EVENT(orangefs_devreq_write_iter,
    TP_PROTO(struct orangefs_kernel_op_s *op),
    TP_ARGS(op),
    TP_STRUCT__entry(
        __array(char, op_name, OP_NAME_LEN)
    ),
    TP_fast_assign(
        strlcpy(__entry->op_name, get_opname_string(op), OP_NAME_LEN);
    ),
    TP_printk(
        "op_name=%s", __entry->op_name
    )
);

TRACE_EVENT(orangefs_early_writeback,
    TP_PROTO(int reason),
    TP_ARGS(reason),
    TP_STRUCT__entry(
        __field(int, reason)
    ),
    TP_fast_assign(
        __entry->reason = reason;
    ),
    TP_printk(
        "%s", __entry->reason == 0 ? "invalidatepage" :
            (__entry->reason == 1 ? "noncontiguous" : "uid/gid")
    )
);

TRACE_EVENT(orangefs_service_operation,
    TP_PROTO(struct orangefs_kernel_op_s *op, int flags),
    TP_ARGS(op, flags),
    TP_STRUCT__entry(
        __array(char, op_name, OP_NAME_LEN)
        __field(int, flags)
        __field(int, attempts)
    ),
    TP_fast_assign(
        strlcpy(__entry->op_name, get_opname_string(op), OP_NAME_LEN);
        __entry->flags = flags;
        __entry->attempts = op->attempts;
    ),
    TP_printk(
        "op_name=%s flags=%d attempts=%d", __entry->op_name, __entry->flags,
        __entry->attempts
    )
);

TRACE_EVENT(orangefs_readpage,
    TP_PROTO(loff_t off, size_t len),
    TP_ARGS(off, len),
    TP_STRUCT__entry(
        __field(loff_t, off)
        __field(size_t, len)
    ),
    TP_fast_assign(
        __entry->off = off;
        __entry->len = len;
    ),
    TP_printk(
        "off=%lld len=%ld", __entry->off, __entry->len
    )
);

TRACE_EVENT(orangefs_writepage,
    TP_PROTO(loff_t off, size_t len, int mwrite),
    TP_ARGS(off, len, mwrite),
    TP_STRUCT__entry(
        __field(loff_t, off)
        __field(size_t, len)
        __field(int, mwrite)
    ),
    TP_fast_assign(
        __entry->off = off;
        __entry->len = len;
        __entry->mwrite = mwrite;
    ),
    TP_printk(
        "off=%lld len=%ld mwrite=%s", __entry->off, __entry->len,
            __entry->mwrite ? "yes" : "no"
    )
);

#endif

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE orangefs-trace
/* This part must be outside protection */
#include <trace/define_trace.h>
