/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 *  lustre/cmm/cmm_split.c
 *  Lustre splitting dir 
 *
 *  Copyright (c) 2006 Cluster File Systems, Inc.
 *   Author: Alex thomas <alex@clusterfs.com>
 *           Wang Di     <wangdi@clusterfs.com>
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

#include <obd_class.h>
#include <lustre_fid.h>
#include <lustre_mds.h>
#include "cmm_internal.h"
#include "mdc_internal.h"

struct cmm_thread_info {
        struct md_attr   cti_ma;
};

struct lu_context_key cmm_thread_key;
struct cmm_thread_info *cmm_ctx_info(const struct lu_context *ctx)
{
        struct cmm_thread_info *info;

        info = lu_context_key_get(ctx, &cmm_thread_key);
        LASSERT(info != NULL);
        return info;
}

#define CMM_NO_SPLIT_EXPECTED   0
#define CMM_EXPECT_SPLIT        1
#define CMM_NO_SPLITTABLE       2

#define SPLIT_SIZE 64*1024

static int cmm_expect_splitting(const struct lu_context *ctx,
                                struct md_object *mo, struct md_attr *ma)
{
        struct cmm_device *cmm = cmm_obj2dev(md2cmm_obj(mo));
        ENTRY;

        if (cmm->cmm_tgt_count == 1)
                RETURN(CMM_NO_SPLIT_EXPECTED);

        if (ma->ma_attr.la_size < SPLIT_SIZE)
                RETURN(CMM_NO_SPLIT_EXPECTED);

        if (ma->ma_lmv_size)
                RETURN(CMM_NO_SPLIT_EXPECTED);
                       
        RETURN(CMM_EXPECT_SPLIT);
}

static inline struct lu_fid* cmm2_fid(struct cmm_object *obj)
{
       return &(obj->cmo_obj.mo_lu.lo_header->loh_fid);
}

#define cmm_md_size(stripes)                            \
       (sizeof(struct lmv_stripe_md) + stripes * sizeof(struct lu_fid))

static int cmm_alloc_fid(const struct lu_context *ctx, struct cmm_device *cmm,
                         struct lu_fid *fid, int count)
{
        struct  mdc_device *mc, *tmp;
        int rc = 0, i = 0;
        
        LASSERT(count == cmm->cmm_tgt_count);
        
        /*FIXME: this spin_lock maybe not proper, 
         * because fid_alloc may need RPC*/
        spin_lock(&cmm->cmm_tgt_guard);
        list_for_each_entry_safe(mc, tmp, &cmm->cmm_targets,
                                 mc_linkage) {
                rc = obd_fid_alloc(mc->mc_desc.cl_exp, &fid[i++], NULL);
                if (rc) {
                        spin_unlock(&cmm->cmm_tgt_guard);
                        RETURN(rc);
                }
        }
        spin_unlock(&cmm->cmm_tgt_guard);
        RETURN(rc);
}

struct cmm_object *cmm_object_find(const struct lu_context *ctxt,
                                   struct cmm_device *d,
                                   const struct lu_fid *f)
{
        struct lu_object *o;
        struct cmm_object *m;
        ENTRY;

        o = lu_object_find(ctxt, d->cmm_md_dev.md_lu_dev.ld_site, f);
        if (IS_ERR(o))
                m = (struct cmm_object *)o;
        else
                m = lu2cmm_obj(lu_object_locate(o->lo_header,
                               d->cmm_md_dev.md_lu_dev.ld_type));
        RETURN(m);
}

static int cmm_creat_remote_obj(const struct lu_context *ctx, 
                                struct cmm_device *cmm,
                                struct lu_fid *fid, struct md_attr *ma)
{
        struct cmm_object *obj;
        struct md_create_spec *spec;
        int rc;
        ENTRY;

        obj = cmm_object_find(ctx, cmm, fid);
        if (IS_ERR(obj))
                RETURN(PTR_ERR(obj));

        OBD_ALLOC_PTR(spec);
        rc = mo_object_create(ctx, md_object_next(&obj->cmo_obj), 
                              spec, ma);
        OBD_FREE_PTR(spec);

        RETURN(0);
}

static int cmm_create_slave_objects(const struct lu_context *ctx,
                                    struct md_object *mo, struct md_attr *ma)
{
        struct cmm_device *cmm = cmm_obj2dev(md2cmm_obj(mo));
        struct lmv_stripe_md *lmv = NULL;
        int lmv_size, i, rc;
        struct lu_fid *lf = cmm2_fid(md2cmm_obj(mo));
        ENTRY;

        lmv_size = cmm_md_size(cmm->cmm_tgt_count + 1);

        OBD_ALLOC(lmv, lmv_size);
        if (!lmv)
                RETURN(-ENOMEM);

        lmv->mea_master = -1; 
        lmv->mea_magic = MEA_MAGIC_ALL_CHARS;
        lmv->mea_count = cmm->cmm_tgt_count + 1;

        lmv->mea_ids[0] = *lf;
        /*create object*/
        rc = cmm_alloc_fid(ctx, cmm, &lmv->mea_ids[1], cmm->cmm_tgt_count);
        if (rc)
                GOTO(cleanup, rc);

        for (i = 0; i < cmm->cmm_tgt_count; i ++) {
                rc = cmm_creat_remote_obj(ctx, cmm, &lmv->mea_ids[i], ma);
                if (rc)
                        GOTO(cleanup, rc);
        }

        rc = mo_xattr_set(ctx, md_object_next(mo), lmv, lmv_size, 
                          MDS_LMV_MD_NAME, 0);
cleanup:
        OBD_FREE(lmv, lmv_size);
        RETURN(rc);
}

static int cmm_scan_and_split(const struct lu_context *ctx,
                              struct md_object *mo, struct md_attr *ma)
{
        RETURN(0);
}

int cml_try_to_split(const struct lu_context *ctx, struct md_object *mo)
{
        struct md_attr *ma;
        int rc = 0;
        ENTRY;

        LASSERT(S_ISDIR(lu_object_attr(&mo->mo_lu)));
       
        OBD_ALLOC_PTR(ma);
        if (ma == NULL)
                RETURN(-ENOMEM);

        ma->ma_need = MA_INODE;
        rc = mo_attr_get(ctx, mo, ma);
        if (rc)
                GOTO(cleanup, ma);

        /*step1: checking whether the dir need to be splitted*/
        rc = cmm_expect_splitting(ctx, mo, ma);
        if (rc != CMM_EXPECT_SPLIT)
                GOTO(cleanup, rc = 0);

        /*step2: create slave objects*/
        rc = cmm_create_slave_objects(ctx, mo, ma);
        if (rc)
                GOTO(cleanup, ma);

        /*step3: scan and split the object*/
        rc = cmm_scan_and_split(ctx, mo, ma);
cleanup:
        OBD_FREE_PTR(ma);

        RETURN(rc);
}

