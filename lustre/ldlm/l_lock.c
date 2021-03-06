/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see
 * http://www.sun.com/software/products/lustre/docs/GPLv2.pdf
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 * GPL HEADER END
 */
/*
 * Copyright (c) 2002, 2010, Oracle and/or its affiliates. All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright (c) 2012, Intel Corporation.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 */

#define DEBUG_SUBSYSTEM S_LDLM
#ifdef __KERNEL__
#include <libcfs/libcfs.h>
#else 
#include <liblustre.h>
#endif

#include <lustre_dlm.h>
#include <lustre_lib.h>

/*
 * ldlm locking uses resource to serialize access to locks
 * but there is a case when we change resource of lock upon
 * enqueue reply. we rely on that lock->l_resource = new_res
 * is atomic
 */
struct ldlm_resource * lock_res_and_lock(struct ldlm_lock *lock)
{
	/* on server-side resource of lock doesn't change */
	if (!lock->l_ns_srv)
		cfs_spin_lock(&lock->l_lock);

	lock_res(lock->l_resource);

	lock->l_res_locked = 1;
	return lock->l_resource;
}

void unlock_res_and_lock(struct ldlm_lock *lock)
{
	/* on server-side resource of lock doesn't change */
	lock->l_res_locked = 0;

	unlock_res(lock->l_resource);
	if (!lock->l_ns_srv)
		cfs_spin_unlock(&lock->l_lock);
}
