/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 *  lustre/mds/handler.c
 *  Lustre Metadata Target (mdt) request handler
 *
 *  Copyright (c) 2006 Cluster File Systems, Inc.
 *   Author: Peter Braam <braam@clusterfs.com>
 *   Author: Andreas Dilger <adilger@clusterfs.com>
 *   Author: Phil Schwan <phil@clusterfs.com>
 *   Author: Mike Shaver <shaver@clusterfs.com>
 *   Author: Nikita Danilov <nikita@clusterfs.com>
 *
 *   This file is part of the Lustre file system, http://www.lustre.org
 *   Lustre is a trademark of Cluster File Systems, Inc.
 *
 *   You may have signed or agreed to another license before downloading
 *   this software.  If so, you are bound by the terms and conditions
 *   of that agreement, and the following does not apply to you.  See the
 *   LICENSE file included with this distribution for more information.
 *
 *   If you did not agree to a different license, then this copy of Lustre
 *   is open source software; you can redistribute it and/or modify it
 *   under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 *
 *   In either case, Lustre is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   license text for more details.
 */

#ifndef EXPORT_SYMTAB
# define EXPORT_SYMTAB
#endif
#define DEBUG_SUBSYSTEM S_MDS

#include <linux/module.h>

/* LUSTRE_VERSION_CODE */
#include <linux/lustre_ver.h>
/*
 * struct OBD_{ALLOC,FREE}*()
 * OBD_FAIL_CHECK
 */
#include <linux/obd_support.h>
/* struct ptlrpc_request */
#include <linux/lustre_net.h>
/* struct obd_export */
#include <linux/lustre_export.h>
/* struct obd_device */
#include <linux/obd.h>

#include <linux/lu_object.h>

/* struct mds_client_data */
#include "../mds/mds_internal.h"
#include "mdt.h"

/*
 * Initialized in mdt_mod_init().
 */
unsigned long mdt_num_threads;

static int mdt_getstatus(struct mdt_thread_info *info,
			 struct ptlrpc_request *req, int offset)
{
        struct md_device *mdd  = info->mti_mdt->mdt_child;
	struct mds_body  *body;
	int               size = sizeof *body;
	int               result;

        ENTRY;

        result = lustre_pack_reply(req, 1, &size, NULL);
	if (result)
                CERROR(LUSTRE_MDT0_NAME" out of memory for message: size=%d\n",
		       size);
        else if (OBD_FAIL_CHECK(OBD_FAIL_MDS_GETSTATUS_PACK))
                result = -ENOMEM;
        else {
		body = lustre_msg_buf(req->rq_repmsg, 0, sizeof *body);
		result = mdd->md_ops->mdo_root_get(mdd, &body->fid1);
	}

        /* the last_committed and last_xid fields are filled in for all
         * replies already - no need to do so here also.
         */
        RETURN(result);
}

static int mdt_connect(struct mdt_thread_info *info,
                       struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_disconnect(struct mdt_thread_info *info,
                          struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_getattr(struct mdt_thread_info *info,
                       struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_getattr_name(struct mdt_thread_info *info,
                            struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_setxattr(struct mdt_thread_info *info,
                        struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_getxattr(struct mdt_thread_info *info,
                        struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_statfs(struct mdt_thread_info *info,
                      struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_readpage(struct mdt_thread_info *info,
                       struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_reint(struct mdt_thread_info *info,
                     struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_close(struct mdt_thread_info *info,
                     struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_done_writing(struct mdt_thread_info *info,
                            struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_pin(struct mdt_thread_info *info,
                   struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_sync(struct mdt_thread_info *info,
                    struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_set_info(struct mdt_thread_info *info,
                        struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_handle_quotacheck(struct mdt_thread_info *info,
                                 struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}

static int mdt_handle_quotactl(struct mdt_thread_info *info,
                               struct ptlrpc_request *req, int offset)
{
        return -EOPNOTSUPP;
}


int fid_lock(const struct ll_fid *f, struct lustre_handle *lh, ldlm_mode_t mode)
{
        return 0;
}

void fid_unlock(const struct ll_fid *f,
                struct lustre_handle *lh, ldlm_mode_t mode)
{
}

static struct lu_device_operations mdt_lu_ops;

static int lu_device_is_mdt(struct lu_device *d)
{
	/*
	 * XXX for now. Tags in lu_device_type->ldt_something are needed.
	 */
	return ergo(d->ld_ops != NULL, d->ld_ops == &mdt_lu_ops);
}

static struct mdt_object *mdt_obj(struct lu_object *o)
{
	LASSERT(lu_device_is_mdt(o->lo_dev));
	return container_of(o, struct mdt_object, mot_obj.mo_lu);
}

struct mdt_object *mdt_object_find(struct mdt_device *d, struct ll_fid *f)
{
	struct lu_object *o;

	o = lu_object_find(d->mdt_md_dev.md_lu_dev.ld_site, f);
	if (IS_ERR(o))
		return (struct mdt_object *)o;
	else
		return mdt_obj(o);
}

void mdt_object_put(struct mdt_object *o)
{
	lu_object_put(&o->mot_obj.mo_lu);
}

struct mdt_handler {
	const char *mh_name;
	int         mh_fail_id;
	__u32       mh_opc;
	__u32       mh_flags;
	int (*mh_act)(struct mdt_thread_info *info,
                      struct ptlrpc_request *req, int offset);
};

enum mdt_handler_flags {
	/*
	 * struct mds_body is passed in the 0-th incoming buffer.
	 */
	HABEO_CORPUS = (1 << 0)
};

struct mdt_opc_slice {
        __u32               mos_opc_start;
        int                 mos_opc_end;
        struct mdt_handler *mos_hs;
};

static struct mdt_opc_slice mdt_handlers[];

struct mdt_handler *mdt_handler_find(__u32 opc)
{
	struct mdt_opc_slice *s;
	struct mdt_handler   *h;

	h = NULL;
	for (s = mdt_handlers; s->mos_hs != NULL; s++) {
		if (s->mos_opc_start <= opc && opc < s->mos_opc_end) {
			h = s->mos_hs + (opc - s->mos_opc_start);
			if (h->mh_opc != 0)
				LASSERT(h->mh_opc == opc);
			else
				h = NULL; /* unsupported opc */
			break;
		}
	}
	return h;
}

static int mdt_req_handle(struct mdt_thread_info *info,
			  struct mdt_handler *h, struct ptlrpc_request *req,
			  int shift)
{
	int result;
        int off;

	ENTRY;

	LASSERT(h->mh_act != NULL);
	LASSERT(h->mh_opc == req->rq_reqmsg->opc);

	DEBUG_REQ(D_INODE, req, "%s", h->mh_name);

	if (h->mh_fail_id != 0)
		OBD_FAIL_RETURN(h->mh_fail_id, 0);

	off = MDS_REQ_REC_OFF + shift;
        result = 0;
	if (h->mh_flags & HABEO_CORPUS) {
		info->mti_body = lustre_swab_reqbuf(req, off,
                                                    sizeof *info->mti_body,
						    lustre_swab_mds_body);
		if (info->mti_body == NULL) {
			CERROR("Can't unpack body\n");
			result = req->rq_status = -EFAULT;
		}
		info->mti_object = mdt_object_find(info->mti_mdt,
						   &info->mti_body->fid1);
		if (IS_ERR(info->mti_object))
			result = PTR_ERR(info->mti_object);
	}
	if (result == 0)
		result = h->mh_act(info, req, off);
	/*
	 * XXX result value is unconditionally shoved into ->rq_status
	 * (original code sometimes placed error code into ->rq_status, and
	 * sometimes returned it to the
	 * caller). ptlrpc_server_handle_request() doesn't check return value
	 * anyway.
	 */
	req->rq_status = result;
	RETURN(result);
}

static void mdt_thread_info_init(struct mdt_thread_info *info)
{
	memset(info, 0, sizeof *info);
	info->mti_fail_id = OBD_FAIL_MDS_ALL_REPLY_NET;
	/*
	 * Poison size array.
	 */
	for (info->mti_rep_buf_nr = 0;
	     info->mti_rep_buf_nr < MDT_REP_BUF_NR_MAX; info->mti_rep_buf_nr++)
		info->mti_rep_buf_size[info->mti_rep_buf_nr] = ~0;
}

static void mdt_thread_info_fini(struct mdt_thread_info *info)
{
	if (info->mti_object != NULL) {
		mdt_object_put(info->mti_object);
		info->mti_object = NULL;
	}
}

static int mds_msg_check_version(struct lustre_msg *msg)
{
        int rc;

        /* TODO: enable the below check while really introducing msg version.
         * it's disabled because it will break compatibility with b1_4.
         */
        return (0);

        switch (msg->opc) {
        case MDS_CONNECT:
        case MDS_DISCONNECT:
        case OBD_PING:
                rc = lustre_msg_check_version(msg, LUSTRE_OBD_VERSION);
                if (rc)
                        CERROR("bad opc %u version %08x, expecting %08x\n",
                               msg->opc, msg->version, LUSTRE_OBD_VERSION);
                break;
        case MDS_GETSTATUS:
        case MDS_GETATTR:
        case MDS_GETATTR_NAME:
        case MDS_STATFS:
        case MDS_READPAGE:
        case MDS_REINT:
        case MDS_CLOSE:
        case MDS_DONE_WRITING:
        case MDS_PIN:
        case MDS_SYNC:
        case MDS_GETXATTR:
        case MDS_SETXATTR:
        case MDS_SET_INFO:
        case MDS_QUOTACHECK:
        case MDS_QUOTACTL:
        case QUOTA_DQACQ:
        case QUOTA_DQREL:
                rc = lustre_msg_check_version(msg, LUSTRE_MDS_VERSION);
                if (rc)
                        CERROR("bad opc %u version %08x, expecting %08x\n",
                               msg->opc, msg->version, LUSTRE_MDS_VERSION);
                break;
        case LDLM_ENQUEUE:
        case LDLM_CONVERT:
        case LDLM_BL_CALLBACK:
        case LDLM_CP_CALLBACK:
                rc = lustre_msg_check_version(msg, LUSTRE_DLM_VERSION);
                if (rc)
                        CERROR("bad opc %u version %08x, expecting %08x\n",
                               msg->opc, msg->version, LUSTRE_DLM_VERSION);
                break;
        case OBD_LOG_CANCEL:
        case LLOG_ORIGIN_HANDLE_CREATE:
        case LLOG_ORIGIN_HANDLE_NEXT_BLOCK:
        case LLOG_ORIGIN_HANDLE_PREV_BLOCK:
        case LLOG_ORIGIN_HANDLE_READ_HEADER:
        case LLOG_ORIGIN_HANDLE_CLOSE:
        case LLOG_CATINFO:
                rc = lustre_msg_check_version(msg, LUSTRE_LOG_VERSION);
                if (rc)
                        CERROR("bad opc %u version %08x, expecting %08x\n",
                               msg->opc, msg->version, LUSTRE_LOG_VERSION);
                break;
        default:
                CERROR("MDS unknown opcode %d\n", msg->opc);
                rc = -ENOTSUPP;
        }
        return rc;
}

static int mdt_handle0(struct ptlrpc_request *req, struct mdt_thread_info *info)
{
        int rc;
        struct mds_obd *mds = NULL; /* quell gcc overwarning */
        struct obd_device *obd = NULL;
	struct mdt_handler *h;

        ENTRY;

        OBD_FAIL_RETURN(OBD_FAIL_MDS_ALL_REQUEST_NET | OBD_FAIL_ONCE, 0);

        LASSERT(current->journal_info == NULL);

        rc = mds_msg_check_version(req->rq_reqmsg);
        if (rc) {
                CERROR(LUSTRE_MDT0_NAME" drops mal-formed request\n");
                RETURN(rc);
        }

        if (req->rq_reqmsg->opc != MDS_CONNECT) {
                struct mds_export_data *med;

                if (req->rq_export == NULL) {
                        CERROR("operation %d on unconnected MDS from %s\n",
                               req->rq_reqmsg->opc,
                               libcfs_id2str(req->rq_peer));
                        req->rq_status = -ENOTCONN;
                        GOTO(out, rc = -ENOTCONN);
                }

                med = &req->rq_export->exp_mds_data;
                obd = req->rq_export->exp_obd;
                mds = &obd->u.mds;

                /* sanity check: if the xid matches, the request must
                 * be marked as a resent or replayed */
                LASSERTF(ergo(req->rq_xid == med->med_mcd->mcd_last_xid,
                              lustre_msg_get_flags(req->rq_reqmsg) &
                              (MSG_RESENT | MSG_REPLAY)),
                         "rq_xid "LPU64" matches last_xid, "
                         "expected RESENT flag\n", req->rq_xid);
                /* else: note the opposite is not always true; a
                 * RESENT req after a failover will usually not match
                 * the last_xid, since it was likely never
                 * committed. A REPLAYed request will almost never
                 * match the last xid, however it could for a
                 * committed, but still retained, open. */

                /* Check for aborted recovery... */
        }

	h = mdt_handler_find(req->rq_reqmsg->opc);
	if (h != NULL) {
		rc = mdt_req_handle(info, h, req, 0);
	} else {
                req->rq_status = -ENOTSUPP;
                rc = ptlrpc_error(req);
                RETURN(rc);
	}

        LASSERT(current->journal_info == NULL);

        /* If we're DISCONNECTing, the mds_export_data is already freed */
        if (!rc && req->rq_reqmsg->opc != MDS_DISCONNECT) {
                struct mds_export_data *med = &req->rq_export->exp_mds_data;
                req->rq_repmsg->last_xid =
                        le64_to_cpu(med->med_mcd->mcd_last_xid);

                target_committed_to_req(req);
        }

        EXIT;
 out:

        if (lustre_msg_get_flags(req->rq_reqmsg) & MSG_LAST_REPLAY) {
                if (obd && obd->obd_recovering) {
                        DEBUG_REQ(D_HA, req, "LAST_REPLAY, queuing reply");
                        RETURN(target_queue_final_reply(req, rc));
                }
                /* Lost a race with recovery; let the error path DTRT. */
                rc = req->rq_status = -ENOTCONN;
        }

        target_send_reply(req, rc, info->mti_fail_id);
	RETURN(0);
}

static struct mdt_device *mdt_dev(struct lu_device *d)
{
	LASSERT(lu_device_is_mdt(d));
	return container_of(d, struct mdt_device, mdt_md_dev.md_lu_dev);
}

int mdt_handle(struct ptlrpc_request *req)
{
	int result;

	struct mdt_thread_info info; /* XXX on stack for now */
	mdt_thread_info_init(&info);
	info.mti_mdt = mdt_dev(req->rq_export->exp_obd->obd_lu_dev);

	result = mdt_handle0(req, &info);

	mdt_thread_info_fini(&info);
        return result;
}

static int mdt_intent_policy(struct ldlm_namespace *ns,
                             struct ldlm_lock **lockp, void *req_cookie,
                             ldlm_mode_t mode, int flags, void *data)
{
	RETURN(ELDLM_LOCK_ABORTED);
}

struct ptlrpc_service *ptlrpc_init_svc_conf(struct ptlrpc_service_conf *c,
					    svc_handler_t h, char *name,
					    struct proc_dir_entry *proc_entry,
					    svcreq_printfn_t prntfn)
{
	return ptlrpc_init_svc(c->psc_nbufs, c->psc_bufsize,
			       c->psc_max_req_size, c->psc_max_reply_size,
			       c->psc_req_portal, c->psc_rep_portal,
			       c->psc_watchdog_timeout,
			       h, name, proc_entry,
			       prntfn, c->psc_num_threads);
}

int md_device_init(struct md_device *md, struct lu_device_type *t)
{
	return lu_device_init(&md->md_lu_dev, t);
}

void md_device_fini(struct md_device *md)
{
	lu_device_fini(&md->md_lu_dev);
}

static void mdt_fini(struct lu_device *d)
{
	struct mdt_device *m = mdt_dev(d);

	if (d->ld_site != NULL) {
		lu_site_fini(d->ld_site);
		d->ld_site = NULL;
	}
	if (m->mdt_service != NULL) {
		ptlrpc_unregister_service(m->mdt_service);
		m->mdt_service = NULL;
	}
	if (m->mdt_namespace != NULL) {
		ldlm_namespace_free(m->mdt_namespace, 0);
		m->mdt_namespace = NULL;
	}
	
	LASSERT(atomic_read(&d->ld_ref) == 0);
	md_device_fini(&m->mdt_md_dev);
}

static int mdt_init0(struct mdt_device *m,
                     struct lu_device_type *t, struct lustre_cfg *cfg)
{
	struct lu_site *s;
        char   ns_name[48];

        ENTRY;

	OBD_ALLOC_PTR(s);
	if (s == NULL)
		return -ENOMEM;

	md_device_init(&m->mdt_md_dev, t);

	m->mdt_md_dev.md_lu_dev.ld_ops = &mdt_lu_ops;

	m->mdt_service_conf.psc_nbufs            = MDS_NBUFS;
	m->mdt_service_conf.psc_bufsize          = MDS_BUFSIZE;
	m->mdt_service_conf.psc_max_req_size     = MDS_MAXREQSIZE;
	m->mdt_service_conf.psc_max_reply_size   = MDS_MAXREPSIZE;
	m->mdt_service_conf.psc_req_portal       = MDS_REQUEST_PORTAL;
	m->mdt_service_conf.psc_rep_portal       = MDC_REPLY_PORTAL;
	m->mdt_service_conf.psc_watchdog_timeout = MDS_SERVICE_WATCHDOG_TIMEOUT;
	/*
	 * We'd like to have a mechanism to set this on a per-device basis,
	 * but alas...
	 */
	m->mdt_service_conf.psc_num_threads = min(max(mdt_num_threads,
                                                      MDT_MIN_THREADS),
						  MDT_MAX_THREADS);
	lu_site_init(s, &m->mdt_md_dev.md_lu_dev);

        snprintf(ns_name, sizeof ns_name, LUSTRE_MDT0_NAME"-%p", m);
        m->mdt_namespace = ldlm_namespace_new(ns_name, LDLM_NAMESPACE_SERVER);
        if (m->mdt_namespace == NULL)
		return -ENOMEM;
        ldlm_register_intent(m->mdt_namespace, mdt_intent_policy);

        ptlrpc_init_client(LDLM_CB_REQUEST_PORTAL, LDLM_CB_REPLY_PORTAL,
                           "mdt_ldlm_client", &m->mdt_ldlm_client);

        m->mdt_service =
                ptlrpc_init_svc_conf(&m->mdt_service_conf, mdt_handle,
                                     LUSTRE_MDT0_NAME,
                                     m->mdt_md_dev.md_lu_dev.ld_proc_entry,
                                     NULL);
	if (m->mdt_service == NULL)
		return -ENOMEM;

	return ptlrpc_start_threads(NULL, m->mdt_service, LUSTRE_MDT0_NAME);
}

struct lu_object *mdt_object_alloc(struct lu_device *d)
{
	struct mdt_object *mo;

	OBD_ALLOC_PTR(mo);
	if (mo != NULL) {
		struct lu_object *o;
		struct lu_object_header *h;

		o = &mo->mot_obj.mo_lu;
		h = &mo->mot_header;
		lu_object_header_init(h);
		lu_object_init(o, h, d);
		/* ->lo_depth and ->lo_flags are automatically 0 */
		lu_object_add_top(h, o);
		return o;
	} else
		return NULL;
}

int mdt_object_init(struct lu_object *o)
{
	struct mdt_device *d = mdt_dev(o->lo_dev);
	struct lu_device  *under;
	struct lu_object  *below;

	under = &d->mdt_child->md_lu_dev;
	below = under->ld_ops->ldo_object_alloc(under);
	if (below != NULL) {
		lu_object_add(o, below);
		return 0;
	} else
		return -ENOMEM;
}

void mdt_object_free(struct lu_object *o)
{
	struct lu_object_header *h;

	h = o->lo_header;
	lu_object_fini(o);
	lu_object_header_fini(h);
}

void mdt_object_release(struct lu_object *o)
{
}

int mdt_object_print(struct seq_file *f, const struct lu_object *o)
{
	return seq_printf(f, LUSTRE_MDT0_NAME"-object@%p", o);
}

static struct lu_device_operations mdt_lu_ops = {
	.ldo_object_alloc   = mdt_object_alloc,
	.ldo_object_init    = mdt_object_init,
	.ldo_object_free    = mdt_object_free,
	.ldo_object_release = mdt_object_release,
	.ldo_object_print   = mdt_object_print
};

static struct ll_fid *mdt_object_fid(struct mdt_object *o)
{
        return lu_object_fid(&o->mot_obj.mo_lu);
}

static int mdt_object_lock(struct mdt_object *o, ldlm_mode_t mode)
{
        return fid_lock(mdt_object_fid(o), &o->mot_lh, mode);
}

static void mdt_object_unlock(struct mdt_object *o, ldlm_mode_t mode)
{
        fid_unlock(mdt_object_fid(o), &o->mot_lh, mode);
}

struct md_object *mdt_object_child(struct mdt_object *o)
{
        return lu2md(lu_object_next(&o->mot_obj.mo_lu));
}

int mdt_mkdir(struct mdt_device *d, struct ll_fid *pfid, const char *name)
{
	struct mdt_object *o;
	int result;

	o = mdt_object_find(d, pfid);
	if (IS_ERR(o))
		return PTR_ERR(o);
	result = mdt_object_lock(o, LCK_PW);
	if (result == 0) {
		result = d->mdt_child->md_ops->mdo_mkdir(mdt_object_child(o),
                                                         name);
		mdt_object_unlock(o, LCK_PW);
	}
	mdt_object_put(o);
	return result;
}

static struct obd_ops mdt_obd_device_ops = {
        .o_owner = THIS_MODULE
};

struct lu_device *mdt_device_alloc(struct lu_device_type *t,
                                   struct lustre_cfg *cfg)
{
        struct lu_device  *l;
        struct mdt_device *m;

        OBD_ALLOC_PTR(m);
        if (m != NULL) {
                int result;

                l = &m->mdt_md_dev.md_lu_dev;
                result = mdt_init0(m, t, cfg);
                if (result != 0) {
                        mdt_fini(l);
                        m = ERR_PTR(result);
                }
        } else
                l = ERR_PTR(-ENOMEM);
        return l;
}

void mdt_device_free(struct lu_device *m)
{
        mdt_fini(m);
        OBD_FREE_PTR(m);
}

int mdt_type_init(struct lu_device_type *t)
{
        return 0;
}

void mdt_type_fini(struct lu_device_type *t)
{
}

static struct lu_device_type_operations mdt_device_type_ops = {
        .ldto_init = mdt_type_init,
        .ldto_fini = mdt_type_fini,

        .ldto_device_alloc = mdt_device_alloc,
        .ldto_device_free  = mdt_device_free
};

static struct lu_device_type mdt_device_type = {
        .ldt_tags = LU_DEVICE_MD,
        .ldt_name = LUSTRE_MDT0_NAME,
        .ldt_ops  = &mdt_device_type_ops
};

struct lprocfs_vars lprocfs_mdt_obd_vars[] = {
        { 0 }
};

struct lprocfs_vars lprocfs_mdt_module_vars[] = {
        { 0 }
};

LPROCFS_INIT_VARS(mdt, lprocfs_mdt_module_vars, lprocfs_mdt_obd_vars);

static int __init mdt_mod_init(void)
{
        struct lprocfs_static_vars lvars;
        struct obd_type *type;
        int result;

        mdt_num_threads = MDT_NUM_THREADS;
        lprocfs_init_vars(mdt, &lvars);
        result = class_register_type(&mdt_obd_device_ops,
                                     lvars.module_vars, LUSTRE_MDT0_NAME);
        if (result == 0) {
                type = class_get_type(LUSTRE_MDT0_NAME);
                LASSERT(type != NULL);
                type->typ_lu = &mdt_device_type;
                result = type->typ_lu->ldt_ops->ldto_init(type->typ_lu);
                if (result != 0)
                        class_unregister_type(LUSTRE_MDT0_NAME);
        }
	return result;
}

static void __exit mdt_mod_exit(void)
{
        class_unregister_type(LUSTRE_MDT0_NAME);
}


#define DEF_HNDL(prefix, base, flags, opc, fn)			\
[prefix ## _ ## opc - prefix ## _ ## base] = {			\
	.mh_name    = #opc,					\
	.mh_fail_id = OBD_FAIL_ ## prefix ## _  ## opc ## _NET,	\
	.mh_opc     = prefix ## _  ## opc,			\
	.mh_flags   = flags,					\
	.mh_act     = fn					\
}

#define DEF_MDT_HNDL(flags, name, fn) DEF_HNDL(MDS, GETATTR, flags, name, fn)

static struct mdt_handler mdt_mds_ops[] = {
	DEF_MDT_HNDL(0,            CONNECT,        mdt_connect),
	DEF_MDT_HNDL(0,            DISCONNECT,     mdt_disconnect),
	DEF_MDT_HNDL(0,            GETSTATUS,      mdt_getstatus),
	DEF_MDT_HNDL(HABEO_CORPUS, GETATTR,        mdt_getattr),
	DEF_MDT_HNDL(HABEO_CORPUS, GETATTR_NAME,   mdt_getattr_name),
	DEF_MDT_HNDL(HABEO_CORPUS, SETXATTR,       mdt_setxattr),
	DEF_MDT_HNDL(HABEO_CORPUS, GETXATTR,       mdt_getxattr),
	DEF_MDT_HNDL(0,            STATFS,         mdt_statfs),
	DEF_MDT_HNDL(HABEO_CORPUS, READPAGE,       mdt_readpage),
	DEF_MDT_HNDL(0,            REINT,          mdt_reint),
	DEF_MDT_HNDL(HABEO_CORPUS, CLOSE,          mdt_close),
	DEF_MDT_HNDL(HABEO_CORPUS, DONE_WRITING,   mdt_done_writing),
	DEF_MDT_HNDL(0,            PIN,            mdt_pin),
	DEF_MDT_HNDL(HABEO_CORPUS, SYNC,           mdt_sync),
	DEF_MDT_HNDL(0,            SET_INFO,       mdt_set_info),
	DEF_MDT_HNDL(0,            QUOTACHECK,     mdt_handle_quotacheck),
	DEF_MDT_HNDL(0,            QUOTACTL,       mdt_handle_quotactl),
};

static struct mdt_handler mdt_obd_ops[] = {
};

static struct mdt_handler mdt_dlm_ops[] = {
};

static struct mdt_handler mdt_llog_ops[] = {
};

static struct mdt_opc_slice mdt_handlers[] = {
	{
		.mos_opc_start = MDS_GETATTR,
		.mos_opc_end   = MDS_LAST_OPC,
		.mos_hs        = mdt_mds_ops
	},
	{
		.mos_opc_start = OBD_PING,
		.mos_opc_end   = OBD_LAST_OPC,
		.mos_hs        = mdt_obd_ops
	},
	{
		.mos_opc_start = LDLM_ENQUEUE,
		.mos_opc_end   = LDLM_LAST_OPC,
		.mos_hs        = mdt_dlm_ops
	},
	{
		.mos_opc_start = LLOG_ORIGIN_HANDLE_CREATE,
		.mos_opc_end   = LLOG_LAST_OPC,
		.mos_hs        = mdt_llog_ops
	},
        {
		.mos_hs        = NULL
        }
};

MODULE_AUTHOR("Cluster File Systems, Inc. <info@clusterfs.com>");
MODULE_DESCRIPTION("Lustre Meta-data Target Prototype ("LUSTRE_MDT0_NAME")");
MODULE_LICENSE("GPL");

CFS_MODULE_PARM(mdt_num_threads, "ul", ulong, 0444,
                "number of mdt service threads to start");

cfs_module(mdt, "0.0.2", mdt_mod_init, mdt_mod_exit);
