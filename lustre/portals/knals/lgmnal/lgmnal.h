/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 *  Copyright (c) 2003 Los Alamos National Laboratory (LANL)
 *
 *   This file is part of Lustre, http://www.lustre.org/
 *
 *   Lustre is free software; you can redistribute it and/or
 *   modify it under the terms of version 2 of the GNU General Public
 *   License as published by the Free Software Foundation.
 *
 *   Lustre is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Lustre; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */



/*
 *	Portals GM kernel NAL header file
 *	This file makes all declaration and prototypes 
 *	for the API side and CB side of the NAL
 */
#ifndef __INCLUDE_LGMNAL_H__
#define __INCLUDE_LGMNAL_H__

#include "linux/config.h"
#include "linux/module.h"
#include "linux/tty.h"
#include "linux/kernel.h"
#include "linux/mm.h"
#include "linux/string.h"
#include "linux/stat.h"
#include "linux/errno.h"
#include "linux/locks.h"
#include "linux/unistd.h"
#include "linux/init.h"
#include "linux/sem.h"
#include "linux/vmalloc.h"
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif

#define DEBUG_SUBSYSTEM S_GMNAL

#include "portals/nal.h"
#include "portals/api.h"
#include "portals/errno.h"
#include "linux/kp30.h"
#include "portals/p30.h"

#include "portals/lib-nal.h"
#include "portals/lib-p30.h"

#define GM_STRONG_TYPES 1
#include "gm.h"
#include "gm_internal.h"


/*
 *	Defines for the API NAL
 */

/*
 *	Small message size is configurable
 *	insmod can set small_msg_size
 *	which is used to populate nal_data.small_msg_size
 */
#define LGMNAL_SMALL_MESSAGE		1078
#define LGMNAL_LARGE_MESSAGE_INIT	1079
#define LGMNAL_LARGE_MESSAGE_ACK	1080
#define LGMNAL_LARGE_MESSAGE_FINI	1081

extern  int lgmnal_small_msg_size;
#define LGMNAL_SMALL_MSG_SIZE(a)		a->small_msg_size
#define LGMNAL_IS_SMALL_MESSAGE(n,a,b,c)	lgmnal_is_small_message(n, a, b, c)
#define LGMNAL_MAGIC				0x1234abcd


/*
 *	Small Transmit Descriptor
 *	A structre to keep track of a small transmit operation
 *	This structure has a one-to-one relationship with a small
 *	transmit buffer (both create by lgmnal_stxd_alloc). 
 *	stxd has pointer to txbuffer and the hash table in nal_data
 *	allows us to go the other way.
 */
typedef struct _lgmnal_stxd_t {
	void 			*buffer;		/* Address of small wired buffer this decriptor uses */
	int			buffer_size;		/* size (in bytes) of the tx buffer this descripto uses */
	gm_size_t		gm_size;		/* gmsize of the tx buffer this descripto uses */
	int			msg_size;		/* size of the message being send */
	int			gm_target_node;
	int			gm_priority;
	int			type;			/* large or small message */
	struct _lgmnal_data_t 	*nal_data;
	lib_msg_t		*cookie;	/* the cookie the portals library gave us */
	int			niov;
	struct iovec		iov[PTL_MD_MAX_IOV];
	struct	_lgmnal_srxd_t  *srxd;	/* for gm_gets */
	struct _lgmnal_stxd_t	*next;
        int                     rxt;    /* this txd for use by rx thread only */
        int                     kniov;
        struct iovec            *iovec_dup;
} lgmnal_stxd_t;

/*
 *	as for lgmnal_stxd_t 
 */
typedef struct _lgmnal_srxd_t {
	void 			*buffer;
	int			size;
	gm_size_t		gmsize;
	unsigned int		gm_source_node;
	lgmnal_stxd_t		*source_stxd;
	int			type;
	int			nsiov;			/* size of iovec sent in LGMNAL_LARGE_MESSAGE_INIT */
	int			nriov;			/* size of iovec used to receive data in LARGE_MESSAGE */
	struct iovec 		*riov;			/* the iovec used to receive data. callback needs it to dergister memory */
	int			ncallbacks;		/* number of gm_get callbacks that have competed */
	spinlock_t		callback_lock;		/* lock provided for ncallbacks */
	int			callback_status;	/* or'd together status of gm_gets */
	lib_msg_t		*cookie;
	struct _lgmnal_srxd_t	*next;
	struct _lgmnal_data_t	*nal_data;
} lgmnal_srxd_t;

/*
 *	Header which lmgnal puts at the start of each message
 */
typedef struct	_lgmnal_msghdr {
	int		magic;
	int 		type;
	unsigned int	sender_node_id;
	lgmnal_stxd_t	*stxd;
	int		niov;
	} lgmnal_msghdr_t;
#define LGMNAL_MSGHDR_SIZE	sizeof(lgmnal_msghdr_t)

/* rxthread work entry */
typedef struct _lgmnal_rxtwe {
	gm_recv_event_t *rx;
	struct _lgmnal_rxtwe	*next;
} lgmnal_rxtwe_t;

/* 
 *	There's one of these for each interface that is initialised
 *	There's a maximum of LGMNAL_NUM_IF lgmnal_data_t
 */

typedef struct _lgmnal_data_t {
	int	refcnt;
	spinlock_t	cb_lock;	/* lock provided for cb_cli function */
	spinlock_t 	stxd_lock;	/* lock to add or remove stxd to/from free list */
	struct semaphore stxd_token;	/* Don't try to access the list until get a token */
	lgmnal_stxd_t	*stxd;		/* list of free stxd's */
	spinlock_t 	rxt_stxd_lock;	/* stxd free list  for the rxthread */
	struct semaphore rxt_stxd_token; /* for rx thread */
	lgmnal_stxd_t	*rxt_stxd;	 /* for rx thread */
	spinlock_t 	srxd_lock;
	struct semaphore srxd_token;
	lgmnal_srxd_t	*srxd;
	struct gm_hash	*srxd_hash;
	nal_t		*nal;		/* our API NAL */
	nal_cb_t	*nal_cb;	/* our CB nal */
	struct gm_port	*gm_port;	/* the gm port structure we open in lgmnal_init */
	unsigned int	gm_local_nid;	/* our gm local node id */
	unsigned int	gm_global_nid;	/* our gm global node id */
	spinlock_t 	gm_lock;	/* GM is not threadsage */
	long		rxthread_pid;	/* pid of our receiver thread */
	long		rxthread_flag;	/* stop the receiver thread */
	long		ctthread_pid;	/* pid of our caretaker thread */
	int		ctthread_flag;	/* stop the thread flag	*/
	gm_alarm_t	ctthread_alarm;	/* used to wake sleeping ct thread */
	int		small_msg_size;
	int		small_msg_gmsize;
	spinlock_t	rxtwe_lock;
	struct	semaphore rxtwe_wait;
	lgmnal_rxtwe_t	*rxtwe_head;
	lgmnal_rxtwe_t	*rxtwe_tail;
	char		_file[128];
	char		_function[128];
	int		_line;
} lgmnal_data_t;

/*
 *	For nal_data->ctthread_flag
 */
#define LGMNAL_THREAD_START	444	
#define LGMNAL_THREAD_STARTED	333
#define LGMNAL_THREAD_CONTINUE	777
#define LGMNAL_THREAD_STOP	666
#define LGMNAL_THREAD_STOPPED	555

#define LGMNAL_NUM_IF 	1

extern lgmnal_data_t	*global_nal_data;

/*
 *	The gm_port to use for lgmnal
 */
#define LGMNAL_GM_PORT	4

/*
 * for ioctl get pid
 */
#define LGMNAL_IOC_GET_GNID 1	

/*
 *	Return codes
 */
#define LGMNAL_STATUS_OK	0
#define LGMNAL_STATUS_FAIL	1
#define LGMNAL_STATUS_NOMEM	2


/*
 *	FUNCTION PROTOTYPES
 */

/*
 *	Locking macros
 */

/*
 *	For the Small tx and rx descriptor lists
 */
#define LGMNAL_TXD_LOCK_INIT(a)			spin_lock_init(&a->stxd_lock);
#define LGMNAL_TXD_LOCK(a)			spin_lock(&a->stxd_lock);
#define LGMNAL_TXD_UNLOCK(a)			spin_unlock(&a->stxd_lock);
#define LGMNAL_TXD_TOKEN_INIT(a, n)		sema_init(&a->stxd_token, n);
#define LGMNAL_TXD_GETTOKEN(a)			down(&a->stxd_token);
#define LGMNAL_TXD_TRYGETTOKEN(a)		down_trylock(&a->stxd_token)
#define LGMNAL_TXD_RETURNTOKEN(a)		up(&a->stxd_token);

#define LGMNAL_RXT_TXD_LOCK_INIT(a)		spin_lock_init(&a->rxt_stxd_lock);
#define LGMNAL_RXT_TXD_LOCK(a)			spin_lock(&a->rxt_stxd_lock);
#define LGMNAL_RXT_TXD_UNLOCK(a)		spin_unlock(&a->rxt_stxd_lock);
#define LGMNAL_RXT_TXD_TOKEN_INIT(a, n)		sema_init(&a->rxt_stxd_token, n);
#define LGMNAL_RXT_TXD_GETTOKEN(a)		down(&a->rxt_stxd_token);
#define LGMNAL_RXT_TXD_TRYGETTOKEN(a)		down_trylock(&a->rxt_stxd_token)
#define LGMNAL_RXT_TXD_RETURNTOKEN(a)		up(&a->rxt_stxd_token);

#define LGMNAL_RXD_LOCK_INIT(a)			spin_lock_init(&a->srxd_lock);
#define LGMNAL_RXD_LOCK(a)			spin_lock(&a->srxd_lock);
#define LGMNAL_RXD_UNLOCK(a)			spin_unlock(&a->srxd_lock);
#define LGMNAL_RXD_TOKEN_INIT(a, n)		sema_init(&a->srxd_token, n);
#define LGMNAL_RXD_GETTOKEN(a)			down(&a->srxd_token);
#define LGMNAL_RXD_TRYGETTOKEN(a)		down_trylock(&a->srxd_token)
#define LGMNAL_RXD_RETURNTOKEN(a)		up(&a->srxd_token);

#define LGMNAL_GM_LOCK_INIT(a)			spin_lock_init(&a->gm_lock);
#define LGMNAL_GM_LOCK(a)			spin_lock(&a->gm_lock);
/*
#define LGMNAL_GM_LOCK(a)			do { \
							while (!spin_trylock(&a->gm_lock)) { \
								CDEBUG(D_INFO, "waiting %s:%s:%d holder %s:%s:%d\n", __FUNCTION__, __FILE__, __LINE__, nal_data->_function, nal_data->_file, nal_data->_line); \
								lgmnal_yield(1); \
							} \
								CDEBUG(D_INFO, "GM Locked %s:%s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
								sprintf(nal_data->_function, "%s", __FUNCTION__); \
								sprintf(nal_data->_file, "%s", __FILE__); \
								nal_data->_line = __LINE__; \
						} while (0)
*/
#define LGMNAL_GM_UNLOCK(a)			spin_unlock(&a->gm_lock);
/*
#define LGMNAL_GM_UNLOCK(a)			do { \
							spin_unlock(&a->gm_lock); \
							memset(nal_data->_function, 0, 128); \
							memset(nal_data->_file, 0, 128); \
							nal_data->_line = 0; \
							CDEBUG(D_INFO, "GM Unlocked %s:%s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
						} while(0);

*/
#define LGMNAL_CB_LOCK_INIT(a)			spin_lock_init(&a->cb_lock);


/*
 *	Memory Allocator
 */

/*
 *	API NAL
 */
int lgmnal_api_forward(nal_t *, int, void *, size_t, void *, size_t);

int lgmnal_api_shutdown(nal_t *, int);

int lgmnal_api_validate(nal_t *, void *, size_t);

void lgmnal_api_yield(nal_t *);

void lgmnal_api_lock(nal_t *, unsigned long *);

void lgmnal_api_unlock(nal_t *, unsigned long *);


#define LGMNAL_INIT_NAL(a)	do { 	\
				a->forward = lgmnal_api_forward; \
				a->shutdown = lgmnal_api_shutdown; \
				a->validate = NULL; \
				a->yield = lgmnal_api_yield; \
				a->lock = lgmnal_api_lock; \
				a->unlock = lgmnal_api_unlock; \
				a->timeout = NULL; \
				a->refct = 1; \
				a->nal_data = NULL; \
				} while (0)


/*
 *	CB NAL
 */

int lgmnal_cb_send(nal_cb_t *, void *, lib_msg_t *, ptl_hdr_t *,
	int, ptl_nid_t, ptl_pid_t, unsigned int, struct iovec *, size_t);

int lgmnal_cb_send_pages(nal_cb_t *, void *, lib_msg_t *, ptl_hdr_t *,
	int, ptl_nid_t, ptl_pid_t, unsigned int, ptl_kiov_t *, size_t);

int lgmnal_cb_recv(nal_cb_t *, void *, lib_msg_t *, 
	unsigned int, struct iovec *, size_t, size_t);

int lgmnal_cb_recv_pages(nal_cb_t *, void *, lib_msg_t *, 
	unsigned int, ptl_kiov_t *, size_t, size_t);

int lgmnal_cb_read(nal_cb_t *, void *private, void *, user_ptr, size_t);

int lgmnal_cb_write(nal_cb_t *, void *private, user_ptr, void *, size_t);

int lgmnal_cb_callback(nal_cb_t *, void *, lib_eq_t *, ptl_event_t *);

void *lgmnal_cb_malloc(nal_cb_t *, size_t);

void lgmnal_cb_free(nal_cb_t *, void *, size_t);

void lgmnal_cb_unmap(nal_cb_t *, unsigned int, struct iovec*, void **);

int  lgmnal_cb_map(nal_cb_t *, unsigned int, struct iovec*, void **); 

void lgmnal_cb_printf(nal_cb_t *, const char *fmt, ...);

void lgmnal_cb_cli(nal_cb_t *, unsigned long *);

void lgmnal_cb_sti(nal_cb_t *, unsigned long *);

int lgmnal_cb_dist(nal_cb_t *, ptl_nid_t, unsigned long *);

nal_t *lgmnal_init(int, ptl_pt_index_t, ptl_ac_index_t, ptl_pid_t rpid);

void  lgmnal_fini(void);



#define LGMNAL_INIT_NAL_CB(a)	do {	\
				a->cb_send = lgmnal_cb_send; \
				a->cb_send_pages = lgmnal_cb_send_pages; \
				a->cb_recv = lgmnal_cb_recv; \
				a->cb_recv_pages = lgmnal_cb_recv_pages; \
				a->cb_read = lgmnal_cb_read; \
				a->cb_write = lgmnal_cb_write; \
				a->cb_callback = lgmnal_cb_callback; \
				a->cb_malloc = lgmnal_cb_malloc; \
				a->cb_free = lgmnal_cb_free; \
				a->cb_map = NULL; \
				a->cb_unmap = NULL; \
				a->cb_printf = lgmnal_cb_printf; \
				a->cb_cli = lgmnal_cb_cli; \
				a->cb_sti = lgmnal_cb_sti; \
				a->cb_dist = lgmnal_cb_dist; \
				a->nal_data = NULL; \
				} while (0)


/*
 *	Small Transmit and Receive Descriptor Functions
 */
int  		lgmnal_alloc_stxd(lgmnal_data_t *);
void 		lgmnal_free_stxd(lgmnal_data_t *);
lgmnal_stxd_t* 	lgmnal_get_stxd(lgmnal_data_t *, int);
void 		lgmnal_return_stxd(lgmnal_data_t *, lgmnal_stxd_t *);

int  		lgmnal_alloc_srxd(lgmnal_data_t *);
void 		lgmnal_free_srxd(lgmnal_data_t *);
lgmnal_srxd_t* 	lgmnal_get_srxd(lgmnal_data_t *, int);
void 		lgmnal_return_srxd(lgmnal_data_t *, lgmnal_srxd_t *);

/*
 *	general utility functions
 */
void 		lgmnal_print(const char *, ...);
lgmnal_srxd_t	*lgmnal_rxbuffer_to_srxd(lgmnal_data_t *, void*);
void		lgmnal_stop_rxthread(lgmnal_data_t *);
void		lgmnal_stop_ctthread(lgmnal_data_t *);
void		lgmnal_small_tx_callback(gm_port_t *, void *, gm_status_t);
void		lgmnal_drop_sends_callback(gm_port_t *, void *, gm_status_t);
char		*lgmnal_gm_error(gm_status_t);
char		*lgmnal_rxevent(gm_recv_event_t*);
int		lgmnal_is_small_message(lgmnal_data_t*, int, struct iovec*, int);
void 		lgmnal_yield(int);


/*
 *	Communication functions
 */

/*
 *	Receive threads
 */
int 		lgmnal_ct_thread(void *); /* caretaker thread */
int 		lgmnal_rx_thread(void *); /* receive thread */
int 		lgmnal_pre_receive(lgmnal_data_t*, gm_recv_t*, int);
int		lgmnal_rx_bad(lgmnal_data_t *, gm_recv_t *, lgmnal_srxd_t *);
int		lgmnal_rx_requeue_buffer(lgmnal_data_t *, lgmnal_srxd_t *);
int		lgmnal_add_rxtwe(lgmnal_data_t *, gm_recv_event_t *);
lgmnal_rxtwe_t * lgmnal_get_rxtwe(lgmnal_data_t *);
void		lgmnal_remove_rxtwe(lgmnal_data_t *);


/*
 *	Small messages
 */
int 		lgmnal_small_rx(nal_cb_t *, void *, lib_msg_t *, unsigned int, struct iovec *, size_t, size_t);
int 		lgmnal_small_tx(nal_cb_t *, void *, lib_msg_t *, ptl_hdr_t *, int, ptl_nid_t, ptl_pid_t, 
				unsigned int, struct iovec*, int);
void 		lgmnal_small_tx_callback(gm_port_t *, void *, gm_status_t);



/*
 *	Large messages
 */
int 		lgmnal_large_rx(nal_cb_t *, void *, lib_msg_t *, unsigned int, struct iovec *, size_t, size_t);
int 		lgmnal_large_tx(nal_cb_t *, void *, lib_msg_t *, ptl_hdr_t *, int, ptl_nid_t, ptl_pid_t, 
				unsigned int, struct iovec*, int);
void 		lgmnal_large_tx_callback(gm_port_t *, void *, gm_status_t);
int 		lgmnal_remote_get(lgmnal_srxd_t *, int, struct iovec*, int, struct iovec*);
void		lgmnal_remote_get_callback(gm_port_t *, void *, gm_status_t);
int 		lgmnal_copyiov(int, lgmnal_srxd_t *, int, struct iovec*, int, struct iovec*);
void 		lgmnal_large_tx_ack(lgmnal_data_t *, lgmnal_srxd_t *);
void 		lgmnal_large_tx_ack_callback(gm_port_t *, void *, gm_status_t);
void 		lgmnal_large_tx_ack_received(lgmnal_data_t *, lgmnal_srxd_t *);

#endif /*__INCLUDE_LGMNAL_H__*/
