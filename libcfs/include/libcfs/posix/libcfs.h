/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
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
 * Copyright  2008 Sun Microsystems, Inc. All rights reserved
 * Use is subject to license terms.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 *
 * libcfs/include/libcfs/posix/libcfs.h
 *
 * Defines for posix userspace.
 *
 * Author: Robert Read <rread@sun.com>
 */

#ifndef __LIBCFS_POSIX_LIBCFS_H__
#define __LIBCFS_POSIX_LIBCFS_H__

#include <errno.h>
#include <sys/errno.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <getopt.h>
#include <signal.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <ctype.h>

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#endif

#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_USER_H
# include <sys/user.h>
#endif

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#include <libcfs/list.h>
#include <libcfs/posix/posix-types.h>
#include <libcfs/user-time.h>
#include <libcfs/user-prim.h>
#include <libcfs/user-mem.h>
#include <libcfs/user-lock.h>
#include <libcfs/user-tcpip.h>
#include <libcfs/posix/posix-wordsize.h>
#include <libcfs/user-bitops.h>
#include <libcfs/posix/posix-kernelcomm.h>

# define do_gettimeofday(tv) gettimeofday(tv, NULL);
typedef unsigned long long cycles_t;

#define IS_ERR(a) ((unsigned long)(a) > (unsigned long)-1000L)
#define PTR_ERR(a) ((long)(a))
#define ERR_PTR(a) ((void*)((long)(a)))

/* this goes in posix-fs.h */
#include <sys/mount.h>

#ifdef __linux__
#include <mntent.h>
#endif

typedef struct file cfs_file_t;
typedef struct dentry cfs_dentry_t;
#ifdef __linux__
typedef struct dirent64 cfs_dirent_t;
#endif

#ifdef __linux__
/* Userpace byte flipping */
# include <endian.h>
# include <byteswap.h>
# define __swab16(x) bswap_16(x)
# define __swab32(x) bswap_32(x)
# define __swab64(x) bswap_64(x)
# define __swab16s(x) do {*(x) = bswap_16(*(x));} while (0)
# define __swab32s(x) do {*(x) = bswap_32(*(x));} while (0)
# define __swab64s(x) do {*(x) = bswap_64(*(x));} while (0)
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define le16_to_cpu(x) (x)
#  define cpu_to_le16(x) (x)
#  define le32_to_cpu(x) (x)
#  define cpu_to_le32(x) (x)
#  define le64_to_cpu(x) (x)
#  define cpu_to_le64(x) (x)

#  define be16_to_cpu(x) bswap_16(x)
#  define cpu_to_be16(x) bswap_16(x)
#  define be32_to_cpu(x) bswap_32(x)
#  define cpu_to_be32(x) bswap_32(x)
#  define be64_to_cpu(x) bswap_64(x)
#  define cpu_to_be64(x) bswap_64(x)
# else
#  if __BYTE_ORDER == __BIG_ENDIAN
#   define le16_to_cpu(x) bswap_16(x)
#   define cpu_to_le16(x) bswap_16(x)
#   define le32_to_cpu(x) bswap_32(x)
#   define cpu_to_le32(x) bswap_32(x)
#   define le64_to_cpu(x) bswap_64(x)
#   define cpu_to_le64(x) bswap_64(x)

#   define be16_to_cpu(x) (x)
#   define cpu_to_be16(x) (x)
#   define be32_to_cpu(x) (x)
#   define cpu_to_be32(x) (x)
#   define be64_to_cpu(x) (x)
#   define cpu_to_be64(x) (x)

#  else
#   error "Unknown byte order"
#  endif /* __BIG_ENDIAN */
# endif /* __LITTLE_ENDIAN */
#elif __APPLE__
#define __cpu_to_le64(x)                        OSSwapHostToLittleInt64(x)
#define __cpu_to_le32(x)                        OSSwapHostToLittleInt32(x)
#define __cpu_to_le16(x)                        OSSwapHostToLittleInt16(x)

#define __le16_to_cpu(x)                        OSSwapLittleToHostInt16(x)
#define __le32_to_cpu(x)                        OSSwapLittleToHostInt32(x)
#define __le64_to_cpu(x)                        OSSwapLittleToHostInt64(x)

#define cpu_to_le64(x)                          __cpu_to_le64(x)
#define cpu_to_le32(x)                          __cpu_to_le32(x)
#define cpu_to_le16(x)                          __cpu_to_le16(x)

#define le64_to_cpu(x)                          __le64_to_cpu(x)
#define le32_to_cpu(x)                          __le32_to_cpu(x)
#define le16_to_cpu(x)                          __le16_to_cpu(x)

#define __swab16(x)                             OSSwapInt16(x)
#define __swab32(x)                             OSSwapInt32(x)
#define __swab64(x)                             OSSwapInt64(x)
#define __swab16s(x)                            do { *(x) = __swab16(*(x)); } while (0)
#define __swab32s(x)                            do { *(x) = __swab32(*(x)); } while (0)
#define __swab64s(x)                            do { *(x) = __swab64(*(x)); } while (0)
#endif


# ifndef THREAD_SIZE /* x86_64 linux has THREAD_SIZE in userspace */
#  define THREAD_SIZE 8192
# endif

#define LUSTRE_TRACE_SIZE (THREAD_SIZE >> 5)

#define CHECK_STACK() do { } while(0)
#define CDEBUG_STACK() (0L)

/* initial pid  */
#define LUSTRE_LNET_PID          12345

#define ENTRY_NESTING_SUPPORT (1)
#define ENTRY_NESTING   do {;} while (0)
#define EXIT_NESTING   do {;} while (0)
#define __current_nesting_level() (0)

/**
 * Platform specific declarations for cfs_curproc API (libcfs/curproc.h)
 *
 * Implementation is in linux-curproc.c
 */
#define CFS_CURPROC_COMM_MAX (sizeof ((struct task_struct *)0)->comm)

typedef __u32 cfs_kernel_cap_t;

/**
 * Module support (probably shouldn't be used in generic code?)
 */
struct module {
        int count;
        char *name;
};

static inline void MODULE_AUTHOR(char *name)
{
        printf("%s\n", name);
}
#define MODULE_DESCRIPTION(name) MODULE_AUTHOR(name)
#define MODULE_LICENSE(name) MODULE_AUTHOR(name)

#define THIS_MODULE (void *)0x11111
#define __init
#define __exit

static inline int request_module(const char *name, ...)
{
        return (-EINVAL);
}

static inline void __module_get(struct module *module)
{
}

static inline int try_module_get(struct module *module)
{
        return 1;
}

static inline void module_put(struct module *module)
{
}


static inline int module_refcount(struct module *m)
{
        return 1;
}

/***************************************************************************
 *
 * Linux kernel slab shrinker emulation. Currently used only in lu_object.c
 *
 ***************************************************************************/

struct shrinker {
        ;
};

#define DEFAULT_SEEKS (0)

typedef int (*shrinker_t)(int, unsigned int);

static inline struct shrinker *set_shrinker(int seeks, shrinker_t shrinkert)
{
        return (struct shrinker *)0xdeadbea1; // Cannot return NULL here
}

static inline void remove_shrinker(struct shrinker *shrinker)
{
}

/***************************************************************************
 *
 * Linux kernel radix tree emulation.
 *
 * XXX this stub-implementation assumes that elements stored in a radix tree
 *     are struct page's and nothing else. Proper implementation will be
 *     committed soon.
 *
 ***************************************************************************/

struct radix_tree_root {
        struct list_head list;
        void *rnode;
};

struct radix_tree_node {
        struct list_head _node;
        unsigned long index;
        void *item;
};

#define RADIX_TREE_INIT(mask)   {               \
                NOT_IMPLEMENTED                 \
}

#define RADIX_TREE(name, mask) \
        struct radix_tree_root name = RADIX_TREE_INIT(mask)


#define INIT_RADIX_TREE(root, mask)                                     \
do {                                                                    \
        CFS_INIT_LIST_HEAD(&((struct radix_tree_root *)root)->list);    \
        ((struct radix_tree_root *)root)->rnode = NULL;                 \
} while (0)

static inline int radix_tree_insert(struct radix_tree_root *root,
                        unsigned long idx, void *item)
{
        struct radix_tree_node *node;
        node = malloc(sizeof(*node));
        if (!node)
                return -ENOMEM;

        CFS_INIT_LIST_HEAD(&node->_node);
        node->index = idx;
        node->item = item;
        list_add_tail(&node->_node, &root->list);
        root->rnode = (void *)1001;
        return 0;
}

static inline struct radix_tree_node *radix_tree_lookup0(struct radix_tree_root *root,
                                      unsigned long idx)
{
        struct radix_tree_node *node;

        if (list_empty(&root->list))
                return NULL;

        cfs_list_for_each_entry_typed(node, &root->list,
                                      struct radix_tree_node, _node)
                if (node->index == idx)
                        return node;

        return NULL;
}

static inline void *radix_tree_lookup(struct radix_tree_root *root,
                                      unsigned long idx)
{
        struct radix_tree_node *node = radix_tree_lookup0(root, idx);

        if (node)
                return node->item;
        return node;
}

static inline void *radix_tree_delete(struct radix_tree_root *root,
                                      unsigned long idx)
{
        struct radix_tree_node *p = radix_tree_lookup0(root, idx);
        void *item;

        if (p == NULL)
                return NULL;

        list_del_init(&p->_node);
        item = p->item;
        free(p);
        if (list_empty(&root->list))
                root->rnode = NULL;

        return item;
}

static inline unsigned int
radix_tree_gang_lookup(struct radix_tree_root *root, void **results,
                       unsigned long first_index, unsigned int max_items)
{
        int i;
        int j = 0;

        for (i = 0; i < max_items; i++, first_index++) {
                results[j++] = radix_tree_lookup(root, first_index);
                if (results[j - 1] == NULL)
                        --j;
        }

        return j;
}

static inline int radix_tree_preload(int gfp_mask)
{
        return 0;
}

void radix_tree_init(void);

static inline void radix_tree_preload_end(void)
{
}

typedef ssize_t (*read_actor_t)();

#define CFS_IFSHIFT 12

#define CFS_IFTODT(type)           (((type) & S_IFMT) >> CFS_IFSHIFT)
#define CFS_DTTOIF(dirtype)        ((dirtype) << CFS_IFSHIFT)

#endif