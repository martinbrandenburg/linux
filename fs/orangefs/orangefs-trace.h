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

#endif

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE orangefs-trace
/* This part must be outside protection */
#include <trace/define_trace.h>
