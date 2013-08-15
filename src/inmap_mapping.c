/*
 * libinputmapper - Mappings
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
 * Mappings
 * For every input device you use, you can create an inmap_mapping object. It
 * contains several tables which allow plain stupid remappings of linux input
 * events. These are meant as fixups for broken kernel mappings or inconsistent
 * cross-device support.
 * All tables can be retrieved via udev's hwdb. Alternatively, an application
 * can create it's own tables.
 */

#include <errno.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include "libinputmapper.h"
#include "libinputmapper_internal.h"

int inmap_mapping_new(struct inmap_ctx *ctx, struct inmap_mapping **out)
{
	struct inmap_mapping *mapping;

	mapping = calloc(1, sizeof(*mapping));
	if (!mapping)
		return -ENOMEM;
	mapping->ref = 1;
	mapping->ctx = ctx;
	inmap_ctx_ref(mapping->ctx);

	*out = mapping;
	return 0;
}

void inmap_mapping_ref(struct inmap_mapping *mapping)
{
	if (!mapping || !mapping->ref)
		return;

	++mapping->ref;
}

void inmap_mapping_unref(struct inmap_mapping *mapping)
{
	unsigned int i;

	if (!mapping || !mapping->ref || --mapping->ref)
		return;

	for (i = 0; i < EV_CNT; ++i)
		free(mapping->tables[i].entries);

	inmap_ctx_unref(mapping->ctx);
	free(mapping);
}

static int inmap_mapping_entry_compare(const void *left, const void *right)
{
	const struct inmap_mapping_entry *l, *r;

	l = left;
	r = right;

	return (__s32)l->from - (__s32)r->from;
}

static struct inmap_mapping_entry*
inmap_mapping_table_find(struct inmap_mapping_table *table, __u16 from)
{
	if (!table->entries || !table->len)
		return NULL;

	return bsearch(&from, table->entries, table->len,
		       sizeof(*table->entries), inmap_mapping_entry_compare);
}

static struct inmap_mapping_entry*
inmap_mapping_find(struct inmap_mapping *mapping, __u16 type, __u16 from)
{
	if (type >= EV_CNT)
		return NULL;

	return inmap_mapping_table_find(&mapping->tables[type], from);
}

void inmap_mapping_map(struct inmap_mapping *mapping,
		       const struct input_event *in,
		       struct input_event *out,
		       enum inmap_mapping_map_flags flags)
{
	struct inmap_mapping_entry *entry;

	if (!in)
		in = out;

	/* NULL is used as special empty mapping */
	if (!mapping)
		goto out_copy;

	entry = inmap_mapping_find(mapping, in->type, in->code);
	if (!entry)
		goto out_copy;

	out->code = entry->to;
	return;

out_copy:
	if (in != out)
		out->code = in->code;
}

int inmap_mapping_add_map(struct inmap_mapping *mapping, __u16 type, __u16 from,
			  __u16 to, enum inmap_mapping_add_flags flags)
{
	struct inmap_mapping_table *table;
	struct inmap_mapping_entry *entry;
	size_t nlen, low, high, mid;

	if (type >= EV_CNT)
		return -EINVAL;

	table = &mapping->tables[type];

	/* resize table if no more free entries available */
	if (table->len == table->size) {
		nlen = table->size ? table->size * 2 : 4;
		entry = realloc(table->entries, sizeof(*table->entries) * nlen);
		if (!entry)
			return -ENOMEM;

		table->entries = entry;
		memset(&entry[table->size], 0,
		       sizeof(*table->entries) * (nlen - table->size));
		table->size = nlen;
	}

	/* insert into sorted array */
	low = 0;
	high = table->len - 1;
	while (low < high) {
		mid = low + (high - low) / 2;
		if (table->entries[mid].from < from)
			low = mid + 1;
		else
			high = mid;
	}

	/* replace existing entry */
	if (table->entries[low].from == from) {
		log_dbg(mapping->ctx, "overwriting existing key %d in mapping",
			(int)from);
		table->entries[low].to = to;
		return 0;
	}

	/* make sure "low" is the new entry-point */
	if (table->entries[low].from < from)
		++low;

	/* insert new entry */
	if (low < table->len)
		memmove(&table->entries[low + 1], &table->entries[low],
			(table->len - low) * sizeof(*table->entries));
	table->entries[low].from = from;
	table->entries[low].to = to;
	++table->len;

	return 0;
}

static int inmap_mapping_parse_type(struct inmap_mapping *mapping,
				    const char *type, __u16 *out)
{
	unsigned long val;
	char *tmp;

	if (!*type) {
		log_dbg(mapping->ctx, "empty type in mapping");
		return -EINVAL;
	}

	/* if it starts with "0x" then parse as hexadecimal number.. */
	if (type[0] == '0' && type[1] == 'x') {
		errno = 0;
		val = strtoul(&type[2], &tmp, 16);
		if (!tmp || *tmp || errno) {
			log_dbg(mapping->ctx, "invalid hex number %s in mapping",
				type);
			return -EINVAL;
		} else if ((unsigned long)(__u16)val != val) {
			log_dbg(mapping->ctx, "integer overflow %lu in mapping",
				val);
			return -EINVAL;
		} else if (val > EV_MAX) {
			log_dbg(mapping->ctx, "type too big (%d > %d) in mapping",
				(int)val, EV_MAX);
			return -EINVAL;
		}

		*out = val;
		return 0;
	}

	/* ..otherwise look it up */

	/* TODO: implement lookup table. Copy from systemd.. */
	return -EINVAL;
}

static int inmap_mapping_parse_code(struct inmap_mapping *mapping, __u16 type,
				    const char *code, __u16 *out)
{
	unsigned long val;
	char *tmp;

	if (!*code) {
		log_dbg(mapping->ctx, "empty code in mapping");
		return -EINVAL;
	}

	/* if it starts with "0x" then parse as hexadecimal number.. */
	if (code[0] == '0' && code[1] == 'x') {
		errno = 0;
		val = strtoul(&code[2], &tmp, 16);
		if (!tmp || *tmp || errno) {
			log_dbg(mapping->ctx, "invalid hex number %s in mapping",
				code);
			return -EINVAL;
		} else if ((unsigned long)(__u16)val != val) {
			log_dbg(mapping->ctx, "integer overflow %lu in mapping",
				val);
			return -EINVAL;
		}

		*out = val;
		return 0;
	}

	/* ..otherwise look it up */

	/* TODO: implement lookup table. Copy from systemd.. */
	return -EINVAL;
}

/*
 * The key consists of the group and code to map separated by an underscore, the
 * value is the target value (same group as key is assumed). Examples:
 *   "KEY_UNKNOWN", "BLUETOOTH" maps KEY_UNKNOWN to KEY_BLUETOOTH
 *   "ABS_X", "ABS_Y" maps ABS_X to ABS_Y
 * Instead of strings, you can also use their hexadecimal value:
 *   "0x1_UNKNOWN", "0x160" maps KEY_UNKNOWN to KEY_OK (0x0001 == EV_KEY,
 *                                                      0x0160 == KEY_OK)
 *   "KEY_0x160", "0x160" maps KEY_UNKNOWN to KEY_UNKNOWN (useless..)
 */
int inmap_mapping_add_map_from_string(struct inmap_mapping *mapping,
				      const char *key, const char *value,
				      enum inmap_mapping_add_flags flags)
{
	const char *type;
	char *k, *code;
	__u16 group, from, to;
	int ret;

	k = strdup(key);
	if (!k)
		return -ENOMEM;

	type = k;
	code = strchr(k, '_');
	if (!code) {
		log_dbg(mapping->ctx, "invalid key %s in mapping", k);
		goto err_key;
	}
	*code++ = 0;

	ret = inmap_mapping_parse_type(mapping, type, &group);
	if (ret)
		goto err_key;

	ret = inmap_mapping_parse_code(mapping, group, code, &from);
	if (ret)
		goto err_key;

	ret = inmap_mapping_parse_code(mapping, group, value, &to);
	if (ret)
		goto err_key;

	free(k);
	return inmap_mapping_add_map(mapping, group, from, to, flags);

err_key:
	free(k);
	return 0;
}
