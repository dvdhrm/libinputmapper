/*
 * libinputmapper - Contexts
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
 * Contexts
 * Every object in libinputmapper is associated to a context. The context tracks
 * global state and logging related information. A single context must not be
 * used simultaneously from multiple threads. However, if you use different
 * contexts in each thread, you can operate on them in parallel.
 */

#include <errno.h>
#include <libudev.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "libinputmapper.h"
#include "libinputmapper_internal.h"

int inmap_ctx_new(struct inmap_ctx **out)
{
	struct inmap_ctx *ctx;
	struct udev *udev;
	int ret;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return -ENOMEM;
	ctx->ref = 1;

	udev = udev_new();
	if (!udev) {
		ret = -ENOMEM;
		goto err_free;
	}

	ctx->hwdb = udev_hwdb_new(udev);
	if (!ctx->hwdb) {
		udev_unref(udev);
		ret = -ENOMEM;
		goto err_free;
	}
	udev_unref(udev);

	*out = ctx;
	return 0;

err_free:
	free(ctx);
	return ret;
}

void inmap_ctx_ref(struct inmap_ctx *ctx)
{
	if (!ctx || !ctx->ref)
		return;

	++ctx->ref;
}

void inmap_ctx_unref(struct inmap_ctx *ctx)
{
	if (!ctx || !ctx->ref || --ctx->ref)
		return;

	udev_hwdb_unref(ctx->hwdb);
	free(ctx);
}

void inmap_ctx_set_log_fn(struct inmap_ctx *ctx, inmap_ctx_log_t log,
			  void *data)
{
	ctx->log = log;
	ctx->log_data = data;
}

void inmap_ctx_vlog(struct inmap_ctx *ctx, const char *file, int line,
		    const char *fn, unsigned int priority, const char *format,
		    va_list args)
{
	if (!ctx->log)
		return;

	ctx->log(ctx->log_data, file, line, fn, priority, format, args);
}

void inmap_ctx_log(struct inmap_ctx *ctx, const char *file, int line,
		   const char *fn, unsigned int priority, const char *format,
		   ...)
{
	va_list list;

	va_start(list, format);
	inmap_ctx_vlog(ctx, file, line, fn, priority, format, list);
	va_end(list);
}

int inmap_ctx_lookup_by_modalias(struct inmap_ctx *ctx, const char *modalias,
				 struct inmap_mapping **out)
{
	struct udev_list_entry *list, *iter;
	struct inmap_mapping *mapping;
	const char *key, *value;
	int ret;

	/* retrieve mappings from udev hwdb */
	list = udev_hwdb_get_properties_list_entry(ctx->hwdb, modalias, 0);

	/* If no mapping exists, we return NULL+success. Other inmap_mapping_*
	 * helpers correctly handle NULL mappings. */
	if (!list) {
		*out = NULL;
		return 0;
	}

	ret = inmap_mapping_new(ctx, &mapping);
	if (ret)
		return ret;

	udev_list_entry_foreach(iter, list) {
		key = udev_list_entry_get_name(iter);
		if (strncmp(key, INMAP_MAPPING_MAP_PREFIX,
				 INMAP_MAPPING_MAP_PREFIX_LEN))
			continue;

		key = &key[INMAP_MAPPING_MAP_PREFIX_LEN];
		value = udev_list_entry_get_value(iter);

		ret = inmap_mapping_add_map_from_string(mapping, key, value,
						INMAP_MAPPING_ADD_DEFAULT);
		if (ret) {
			inmap_mapping_unref(mapping);
			return ret;
		}
	}

	*out = mapping;
	return 0;
}
