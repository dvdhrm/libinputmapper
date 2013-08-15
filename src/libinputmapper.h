/*
 * libinputmapper - Main Header
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
 * libinputmapper
 * This library provides translation tables for linux input events. It can read
 * simple one-to-one mappings from udev's hwdb and assembles lookup tables for
 * fast event translations.
 * The purpose is to fixup broken device keymaps or cross-device
 * incompatibilities.
 */

#ifndef INMAP_H
#define INMAP_H

#include <linux/input.h>
#include <stdarg.h>
#include <stdlib.h>

struct inmap_ctx;
struct inmap_mapping;

/* contexts */

int inmap_ctx_new(struct inmap_ctx **out);
void inmap_ctx_ref(struct inmap_ctx *ctx);
void inmap_ctx_unref(struct inmap_ctx *ctx);

typedef void (*inmap_ctx_log_t) (void *data, const char *file, int line,
				 const char *fn, unsigned int priority,
				 const char *format, va_list args);

void inmap_ctx_set_log_fn(struct inmap_ctx *ctx, inmap_ctx_log_t log,
			  void *data);

int inmap_ctx_lookup_by_modalias(struct inmap_ctx *ctx, const char *modalias,
				 struct inmap_mapping **out);

/* mappings */

int inmap_mapping_new(struct inmap_ctx *ctx, struct inmap_mapping **out);
void inmap_mapping_ref(struct inmap_mapping *mapping);
void inmap_mapping_unref(struct inmap_mapping *mapping);

enum inmap_mapping_map_flags {
	INMAP_MAPPING_MAP_DEFAULT		= 0,
};

void inmap_mapping_map(struct inmap_mapping *mapping,
		       const struct input_event *in,
		       struct input_event *out,
		       enum inmap_mapping_map_flags flags);

enum inmap_mapping_add_flags {
	INMAP_MAPPING_ADD_DEFAULT		= 0,
};

int inmap_mapping_add_map(struct inmap_mapping *mapping, __u16 type, __u16 from,
			  __u16 to, enum inmap_mapping_add_flags flags);
int inmap_mapping_add_map_from_string(struct inmap_mapping *mapping,
				      const char *key, const char *value,
				      enum inmap_mapping_add_flags flags);

#endif /* INMAP_H */
