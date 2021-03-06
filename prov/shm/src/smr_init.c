/*
 * Copyright (c) 2015-2017 Intel Corporation. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <rdma/fi_errno.h>

#include <ofi_prov.h>
#include "smr.h"


static void smr_resolve_addr(const char *node, const char *service,
			     char **addr, size_t *addrlen)
{
	char temp_name[SMR_NAME_SIZE];

	if (service) {
		if (node)
			snprintf(temp_name, SMR_NAME_SIZE, "%s%s:%s",
				 SMR_PREFIX_NS, node, service);
		else
			snprintf(temp_name, SMR_NAME_SIZE, "%s%s",
				 SMR_PREFIX_NS, service);
	} else {
		if (node)
			snprintf(temp_name, SMR_NAME_SIZE, "%s%s",
				 SMR_PREFIX, node);
		else
			snprintf(temp_name, SMR_NAME_SIZE, "%s%d",
				 SMR_PREFIX, getpid());
	}

	*addr = strdup(temp_name);
	*addrlen = strlen(*addr);
}

static int smr_getinfo(uint32_t version, const char *node, const char *service,
		       uint64_t flags, const struct fi_info *hints,
		       struct fi_info **info)
{
	struct fi_info *cur;
	int ret;

	ret = util_getinfo(&smr_util_prov, version, node, service, flags,
			   hints, info);
	if (ret)
		return ret;

	for (cur = *info; cur; cur = cur->next) {
		if (!(flags & FI_SOURCE) && !cur->dest_addr)
			smr_resolve_addr(node, service, (char **) &cur->dest_addr,
					 &cur->dest_addrlen);

		if (!cur->src_addr) {
			if (flags & FI_SOURCE)
				smr_resolve_addr(node, service, (char **) &cur->src_addr,
						 &cur->src_addrlen);
			else
				smr_resolve_addr(NULL, NULL, (char **) &cur->src_addr,
						 &cur->src_addrlen);
		}
	}
	return 0;
}

static void smr_fini(void)
{
	/* yawn */
}

struct fi_provider smr_prov = {
	.name = "shm",
	.version = FI_VERSION(SMR_MAJOR_VERSION, SMR_MINOR_VERSION),
	.fi_version = FI_VERSION(1, 6),
	.getinfo = smr_getinfo,
	.fabric = smr_fabric,
	.cleanup = smr_fini
};

struct util_prov smr_util_prov = {
	.prov = &smr_prov,
	.info = &smr_info,
	.flags = 0
};

SHM_INI
{
	return &smr_prov;
}
