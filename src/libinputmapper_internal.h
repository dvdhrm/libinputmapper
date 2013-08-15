/*
 * libinputmapper - Internal Header
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Internal Header
 * Internal definitions which are implementation specific and should never be
 * installed into the system. Not part of the ABI.
 */

#ifndef INMAP_INTERNAL_H
#define INMAP_INTERNAL_H

#include <libudev.h>
#include <linux/input.h>
#include <stdarg.h>
#include <stdlib.h>
#include "libinputmapper.h"

/* contexts */

struct inmap_ctx {
	unsigned long ref;

	inmap_ctx_log_t log;
	void *log_data;

	struct udev_hwdb *hwdb;
};

void inmap_ctx_vlog(struct inmap_ctx *ctx, const char *file, int line,
		    const char *fn, unsigned int priority, const char *format,
		    va_list args);
void inmap_ctx_log(struct inmap_ctx *ctx, const char *file, int line,
		   const char *fn, unsigned int priority, const char *format,
		   ...);

#define inmap_log(ctx, priority, format, ...) inmap_ctx_log((ctx), __FILE__, \
		__LINE__, __func__, (priority), (format), ##__VA_ARGS__)
#define log_warn(ctx, format, ...) inmap_log((ctx), 4, (format), ##__VA_ARGS__)
#define log_dbg(ctx, format, ...) inmap_log((ctx), 7, (format), ##__VA_ARGS__)

/* mappings */

#define INMAP_MAPPING_MAP_PREFIX "INMAP_MAP_"
#define INMAP_MAPPING_MAP_PREFIX_LEN (sizeof(INMAP_MAPPING_MAP_PREFIX) - 1)

struct inmap_mapping_entry {
	__u16 from;
	__u16 to;
};

struct inmap_mapping_table {
	size_t len;
	size_t size;
	struct inmap_mapping_entry *entries;
};

struct inmap_mapping {
	unsigned long ref;
	struct inmap_ctx *ctx;

	struct inmap_mapping_table tables[EV_CNT];
};

#endif /* INMAP_INTERNAL_H */
