/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef MPIDPRE_H_INCLUDED
#define MPIDPRE_H_INCLUDED

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "mpir_dataloop.h"
#ifdef HAVE_LIBHCOLL
#include "hcoll/api/hcoll_dte.h"
#endif

#include "mpid_thread.h"
#include "mpid_sched.h"
#include "mpid_timers_fallback.h"
#include "netmodpre.h"
#include "shmpre.h"
#include "uthash.h"
#include "ch4_coll_params.h"
#include "ch4i_workq_types.h"

#ifdef MPIDI_CH4_USE_MT_DIRECT
#define MPIDI_CH4_MT_MODEL MPIDI_CH4_MT_DIRECT
#elif defined MPIDI_CH4_USE_MT_HANDOFF
#define MPIDI_CH4_MT_MODEL MPIDI_CH4_MT_HANDOFF
#elif defined MPIDI_CH4_USE_MT_TRYLOCK
#define MPIDI_CH4_MT_MODEL MPIDI_CH4_MT_TRYLOCK
#elif defined MPIDI_CH4_USE_MT_RUNTIME
#define MPIDI_CH4_MT_MODEL MPIDI_CH4_Global.settings.mt_model
#else
#error "Unknown MT model or MT model not defined"
#endif

typedef struct {
#ifdef HAVE_LIBHCOLL
    hcoll_datatype_t hcoll_datatype;
#endif
    union {
    MPIDI_NM_DT_DECL} netmod;
} MPIDI_Devdt_t;
#define MPID_DEV_DATATYPE_DECL   MPIDI_Devdt_t   dev;

typedef struct {
    int progress_count;
} MPID_Progress_state;

#define CH4_COMPILE_TIME_ASSERT(expr_)                                  \
  do { switch(0) { case 0: case (expr_): default: break; } } while (0)

typedef enum {
    MPIDI_PTYPE_RECV,
    MPIDI_PTYPE_SEND,
    MPIDI_PTYPE_BSEND,
    MPIDI_PTYPE_SSEND
} MPIDI_ptype;

#define MPIDI_CH4U_REQ_BUSY           (0x1)
#define MPIDI_CH4U_REQ_PEER_SSEND     (0x1 << 1)
#define MPIDI_CH4U_REQ_UNEXPECTED     (0x1 << 2)
#define MPIDI_CH4U_REQ_UNEXP_DQUED    (0x1 << 3)
#define MPIDI_CH4U_REQ_UNEXP_CLAIMED  (0x1 << 4)
#define MPIDI_CH4U_REQ_RCV_NON_CONTIG (0x1 << 5)
#define MPIDI_CH4U_REQ_MATCHED (0x1 << 6)
#define MPIDI_CH4U_REQ_LONG_RTS (0x1 << 7)

#define MPIDI_PARENT_PORT_KVSKEY "PARENT_ROOT_PORT_NAME"
#define MPIDI_MAX_KVS_VALUE_LEN  4096

typedef struct MPIDI_CH4U_sreq_t {
    /* persistent send fields */
} MPIDI_CH4U_sreq_t;

typedef struct MPIDI_CH4U_lreq_t {
    /* Long send fields */
    const void *src_buf;
    MPI_Count count;
    MPI_Datatype datatype;
    int rank;
    int tag;
    MPIR_Context_id_t context_id;
} MPIDI_CH4U_lreq_t;

typedef struct MPIDI_CH4U_rreq_t {
    /* mrecv fields */
    void *mrcv_buffer;
    uint64_t mrcv_count;
    MPI_Datatype mrcv_datatype;

    uint64_t ignore;
    uint64_t peer_req_ptr;
    uint64_t match_req;
    uint64_t request;

    struct MPIDI_CH4U_rreq_t *prev, *next;
} MPIDI_CH4U_rreq_t;

typedef struct MPIDI_CH4U_put_req_t {
    MPIR_Win *win_ptr;
    uint64_t preq_ptr;
    void *dt_iov;
    void *origin_addr;
    int origin_count;
    MPI_Datatype origin_datatype;
    int n_iov;
} MPIDI_CH4U_put_req_t;

typedef struct MPIDI_CH4U_get_req_t {
    MPIR_Win *win_ptr;
    uint64_t greq_ptr;
    uint64_t addr;
    MPI_Datatype datatype;
    int count;
    int n_iov;
    void *dt_iov;
} MPIDI_CH4U_get_req_t;

typedef struct MPIDI_CH4U_cswap_req_t {
    MPIR_Win *win_ptr;
    uint64_t creq_ptr;
    uint64_t addr;
    MPI_Datatype datatype;
    void *data;
    void *result_addr;
} MPIDI_CH4U_cswap_req_t;

typedef struct MPIDI_CH4U_acc_req_t {
    MPIR_Win *win_ptr;
    uint64_t req_ptr;
    MPI_Datatype origin_datatype;
    MPI_Datatype target_datatype;
    int origin_count;
    int target_count;
    int n_iov;
    void *target_addr;
    void *dt_iov;
    void *data;
    size_t data_sz;
    MPI_Op op;
    void *result_addr;
    int result_count;
    void *origin_addr;
    MPI_Datatype result_datatype;
} MPIDI_CH4U_acc_req_t;

typedef struct MPIDI_CH4U_req_ext_t {
    union {
        MPIDI_CH4U_sreq_t sreq;
        MPIDI_CH4U_lreq_t lreq;
        MPIDI_CH4U_rreq_t rreq;
        MPIDI_CH4U_put_req_t preq;
        MPIDI_CH4U_get_req_t greq;
        MPIDI_CH4U_cswap_req_t creq;
        MPIDI_CH4U_acc_req_t areq;
    };

    struct iovec *iov;
    void *target_cmpl_cb;
    uint64_t seq_no;
    uint64_t request;
    uint64_t status;
    struct MPIDI_CH4U_req_ext_t *next, *prev;

} MPIDI_CH4U_req_ext_t;

typedef struct MPIDI_CH4U_req_t {
    union {
    MPIDI_NM_REQUEST_AM_DECL} netmod_am;
    MPIDI_CH4U_req_ext_t *req;
    MPIDI_ptype p_type;         /* persistent request type */
    void *buffer;
    uint64_t count;
    int rank;
    int tag;
    MPIR_Context_id_t context_id;
    MPI_Datatype datatype;
} MPIDI_CH4U_req_t;

typedef struct {
#ifndef MPIDI_CH4_DIRECT_NETMOD
    int is_local;
#endif
    /* Anysource handling. Netmod and shm specific requests are cross
     * referenced. This must be present all of the time to avoid lots of extra
     * ifdefs in the code. */
#ifndef MPIDI_CH4_DIRECT_NETMOD
    struct MPIR_Request *anysource_partner_request;
#endif

    union {
        /* The first fields are used by the CH4U apis */
        MPIDI_CH4U_req_t am;

        /* Used by the netmod direct apis */
        union {
        MPIDI_NM_REQUEST_DECL} netmod;

        union {
        MPIDI_SHM_REQUEST_DECL} shm;

#ifdef MPIDI_CH4_USE_WORK_QUEUES
        MPIDI_workq_elemt_t command;
#endif
    } ch4;
} MPIDI_Devreq_t;
#define MPIDI_REQUEST_HDR_SIZE              offsetof(struct MPIR_Request, dev.ch4.netmod)
#define MPIDI_CH4I_REQUEST(req,field)       (((req)->dev).field)
#define MPIDI_CH4U_REQUEST(req,field)       (((req)->dev.ch4.am).field)

#ifndef MPIDI_CH4_DIRECT_NETMOD
#define MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req)  (((req)->dev).anysource_partner_request)
#else
#define MPIDI_CH4I_REQUEST_ANYSOURCE_PARTNER(req)  NULL
#endif

MPL_STATIC_INLINE_PREFIX void MPID_Request_create_hook(struct MPIR_Request *req);
MPL_STATIC_INLINE_PREFIX void MPID_Request_free_hook(struct MPIR_Request *req);
MPL_STATIC_INLINE_PREFIX void MPID_Request_destroy_hook(struct MPIR_Request *req);

typedef struct MPIDI_CH4U_win_shared_info {
    uint32_t disp_unit;
    size_t size;
    void *shm_base_addr;
} MPIDI_CH4U_win_shared_info_t;

#define MPIDI_CH4I_ACCU_ORDER_RAR (1)
#define MPIDI_CH4I_ACCU_ORDER_RAW (1 << 1)
#define MPIDI_CH4I_ACCU_ORDER_WAR (1 << 2)
#define MPIDI_CH4I_ACCU_ORDER_WAW (1 << 3)

typedef enum {
    MPIDI_CH4I_ACCU_SAME_OP,
    MPIDI_CH4I_ACCU_SAME_OP_NO_OP
} MPIDI_CH4U_win_info_accumulate_ops;

typedef enum {
    MPIDI_CH4I_ACCU_MAX_SHIFT = 0,      /* 1<<0 */
    MPIDI_CH4I_ACCU_MIN_SHIFT = 1,
    MPIDI_CH4I_ACCU_SUM_SHIFT = 2,
    MPIDI_CH4I_ACCU_PROD_SHIFT = 3,
    MPIDI_CH4I_ACCU_MAXLOC_SHIFT = 4,
    MPIDI_CH4I_ACCU_MINLOC_SHIFT = 5,
    MPIDI_CH4I_ACCU_BAND_SHIFT = 6,
    MPIDI_CH4I_ACCU_BOR_SHIFT = 7,
    MPIDI_CH4I_ACCU_BXOR_SHIFT = 8,
    MPIDI_CH4I_ACCU_LAND_SHIFT = 9,
    MPIDI_CH4I_ACCU_LOR_SHIFT = 10,
    MPIDI_CH4I_ACCU_LXOR_SHIFT = 11,
    MPIDI_CH4I_ACCU_REPLACE_SHIFT = 12,
    MPIDI_CH4I_ACCU_NO_OP_SHIFT = 13,   /* atomic get */
    MPIDI_CH4I_ACCU_CSWAP_SHIFT = 14,
    MPIDI_CH4I_ACCU_OP_SHIFT_LAST
} MPIDI_CH4U_win_info_accu_op_shift_t;

typedef struct MPIDI_CH4U_win_info_args_t {
    int no_locks;
    int same_size;
    int same_disp_unit;
    int accumulate_ordering;
    int alloc_shared_noncontig;
    MPIDI_CH4U_win_info_accumulate_ops accumulate_ops;

    /* hints to tradeoff atomicity support */
    uint32_t which_accumulate_ops;      /* Arbitrary combination of {1<<max|1<<min|1<<sum|...}
                                         * with bit shift defined in MPIDI_CH4U_win_info_accu_op_shift_t.
                                         * any_op and none are two special values.
                                         * any_op by default. */
    bool accumulate_noncontig_dtype;    /* true by default. */
    MPI_Aint accumulate_max_bytes;      /* Non-negative integer, -1 (unlimited) by default.
                                         * TODO: can be set to win_size.*/
    bool disable_shm_accumulate;        /* false by default. */

    /* alloc_shm: MPICH specific hint (same in CH3).
     * If true, MPICH will try to use shared memory routines for the window.
     * Default is true for allocate-based windows, and false for other
     * windows. Note that this hint can be also used in create-based windows,
     * and it means the user window buffer is allocated over shared memory,
     * thus RMA operation can use shm routines. */
    int alloc_shm;
} MPIDI_CH4U_win_info_args_t;

struct MPIDI_CH4U_win_lock {
    struct MPIDI_CH4U_win_lock *next;
    int rank;
    uint16_t mtype;
    uint16_t type;
};

typedef struct MPIDI_CH4U_win_lock_recvd {
    struct MPIDI_CH4U_win_lock *head;
    struct MPIDI_CH4U_win_lock *tail;
    int type;                   /* current lock's type */
    unsigned count;             /* count of granted locks (not received) */
} MPIDI_CH4U_win_lock_recvd_t;

typedef struct MPIDI_CH4U_win_target_sync_lock {
    /* NOTE: use volatile to avoid compiler optimization which keeps reading
     * register value when no dependency or function pointer is found in fully
     * inlined code.*/
    volatile unsigned locked;   /* locked == 0 or 1 */
} MPIDI_CH4U_win_target_sync_lock_t;

typedef struct MPIDI_CH4U_win_sync_lock {
    unsigned count;             /* count of lock epochs on the window */
} MPIDI_CH4U_win_sync_lock_t;

typedef struct MPIDI_CH4U_win_sync_lockall {
    /* NOTE: use volatile to avoid compiler optimization which keeps reading
     * register value when no dependency or function pointer is found in fully
     * inlined code.*/
    volatile unsigned allLocked;        /* 0 <= allLocked < size */
} MPIDI_CH4U_win_sync_lockall_t;

typedef struct MPIDI_CH4U_win_sync_pscw {
    struct MPIR_Group *group;
    /* NOTE: use volatile to avoid compiler optimization which keeps reading
     * register value when no dependency or function pointer is found in fully
     * inlined code.*/
    volatile unsigned count;
} MPIDI_CH4U_win_sync_pscw_t;

typedef struct MPIDI_CH4U_win_target_sync {
    int access_epoch_type;      /* NONE, LOCK. */
    MPIDI_CH4U_win_target_sync_lock_t lock;
    uint32_t assert_mode;       /* bit-vector OR of zero or more of the following integer constant:
                                 * MPI_MODE_NOCHECK, MPI_MODE_NOSTORE, MPI_MODE_NOPUT, MPI_MODE_NOPRECEDE, MPI_MODE_NOSUCCEED. */
} MPIDI_CH4U_win_target_sync_t;

typedef struct MPIDI_CH4U_win_sync {
    int access_epoch_type;      /* NONE, FENCE, LOCKALL, START,
                                 * LOCK (refer to target_sync). */
    int exposure_epoch_type;    /* NONE, FENCE, POST. */
    uint32_t assert_mode;       /* bit-vector OR of zero or more of the following integer constant:
                                 * MPI_MODE_NOCHECK, MPI_MODE_NOSTORE, MPI_MODE_NOPUT, MPI_MODE_NOPRECEDE, MPI_MODE_NOSUCCEED. */

    /* access epochs */
    /* TODO: Can we put access epochs in union,
     * since no concurrent epochs is allowed ? */
    MPIDI_CH4U_win_sync_pscw_t sc;
    MPIDI_CH4U_win_sync_lockall_t lockall;
    MPIDI_CH4U_win_sync_lock_t lock;

    /* exposure epochs */
    MPIDI_CH4U_win_sync_pscw_t pw;
    MPIDI_CH4U_win_lock_recvd_t lock_recvd;
} MPIDI_CH4U_win_sync_t;

typedef struct MPIDI_CH4U_win_target {
    MPIR_cc_t local_cmpl_cnts;  /* increase at OP issuing, decrease at local completion */
    MPIR_cc_t remote_cmpl_cnts; /* increase at OP issuing, decrease at remote completion */
    MPIR_cc_t remote_acc_cmpl_cnts;     /* for acc only, increase at OP issuing, decrease at remote completion */
    MPIDI_CH4U_win_target_sync_t sync;
    int rank;
    UT_hash_handle hash_handle;
} MPIDI_CH4U_win_target_t;

typedef struct MPIDI_CH4U_win_t {
    uint64_t win_id;
    void *mmap_addr;
    int64_t mmap_sz;

    MPL_shm_hnd_t shm_segment_handle;

    /* per-window OP completion for fence */
    MPIR_cc_t local_cmpl_cnts;  /* increase at OP issuing, decrease at local completion */
    MPIR_cc_t remote_cmpl_cnts; /* increase at OP issuing, decrease at remote completion */
    MPIR_cc_t remote_acc_cmpl_cnts;     /* for acc only, increase at OP issuing, decrease at remote completion */

    MPIDI_CH4U_win_sync_t sync;
    MPIDI_CH4U_win_info_args_t info_args;
    MPIDI_CH4U_win_shared_info_t *shared_table;
    unsigned shm_allocated;     /* shm optimized flag (0 or 1), set at shmmod win initialization time.
                                 * Equal to 1 if the window has a shared memory region associated with it
                                 * and the shmmod supports load/store based RMA operations over the window
                                 * (e.g., may rely on support of interprocess mutex). */

    /* per-target structure for sync and OP completion. */
    MPIDI_CH4U_win_target_t *targets;
} MPIDI_CH4U_win_t;

typedef struct {
    MPIDI_CH4U_win_t ch4u;
    union {
    MPIDI_NM_WIN_DECL} netmod;
    struct {
        /* multiple shmmods may co-exist. */
    MPIDI_SHM_WIN_DECL} shm;
} MPIDI_Devwin_t;

#define MPIDI_CH4U_WIN(win,field)        (((win)->dev.ch4u).field)
#define MPIDI_CH4U_WINFO(win,rank) (MPIDI_CH4U_win_info_t*) &(MPIDI_CH4U_WIN(win, info_table)[rank])

typedef unsigned MPIDI_locality_t;

typedef struct MPIDI_CH4U_comm_t {
    MPIDI_CH4U_rreq_t *posted_list;
    MPIDI_CH4U_rreq_t *unexp_list;
    uint32_t window_instance;
} MPIDI_CH4U_comm_t;

#define MPIDI_CALC_STRIDE(rank, stride, blocksize, offset) \
    ((rank) / (blocksize) * ((stride) - (blocksize)) + (rank) + (offset))

#define MPIDI_CALC_STRIDE_SIMPLE(rank, stride, offset) \
    ((rank) * (stride) + (offset))

typedef enum {
    MPIDI_RANK_MAP_DIRECT,
    MPIDI_RANK_MAP_DIRECT_INTRA,
    MPIDI_RANK_MAP_OFFSET,
    MPIDI_RANK_MAP_OFFSET_INTRA,
    MPIDI_RANK_MAP_STRIDE,
    MPIDI_RANK_MAP_STRIDE_INTRA,
    MPIDI_RANK_MAP_STRIDE_BLOCK,
    MPIDI_RANK_MAP_STRIDE_BLOCK_INTRA,
    MPIDI_RANK_MAP_LUT,
    MPIDI_RANK_MAP_LUT_INTRA,
    MPIDI_RANK_MAP_MLUT,
    MPIDI_RANK_MAP_NONE
} MPIDI_rank_map_mode;

typedef int MPIDI_lpid_t;
typedef struct {
    int avtid;
    int lpid;
} MPIDI_gpid_t;

typedef struct {
    MPIR_OBJECT_HEADER;
    MPIDI_lpid_t lpid[0];
} MPIDI_rank_map_lut_t;

typedef struct {
    MPIR_OBJECT_HEADER;
    MPIDI_gpid_t gpid[0];
} MPIDI_rank_map_mlut_t;

typedef struct {
    MPIDI_rank_map_mode mode;
    int avtid;
    int size;

    union {
        int offset;
        struct {
            int offset;
            int stride;
            int blocksize;
        } stride;
    } reg;

    union {
        struct {
            MPIDI_rank_map_lut_t *t;
            MPIDI_lpid_t *lpid;
        } lut;
        struct {
            MPIDI_rank_map_mlut_t *t;
            MPIDI_gpid_t *gpid;
        } mlut;
    } irreg;
} MPIDI_rank_map_t;

typedef struct MPIDI_Devcomm_t {
    struct {
        /* The first fields are used by the CH4U apis */
        MPIDI_CH4U_comm_t ch4u;

        /* Used by the netmod direct apis */
        union {
        MPIDI_NM_COMM_DECL} netmod;

        union {
        MPIDI_SHM_COMM_DECL} shm;

        MPIDI_rank_map_t map;
        MPIDI_rank_map_t local_map;
    } ch4;
} MPIDI_Devcomm_t;
#define MPIDI_CH4U_COMM(comm,field) ((comm)->dev.ch4.ch4u).field
#define MPIDI_COMM(comm,field) ((comm)->dev.ch4).field

typedef struct {
    union {
    MPIDI_NM_OP_DECL} netmod;
} MPIDI_Devop_t;

#define MPID_DEV_REQUEST_DECL    MPIDI_Devreq_t  dev;
#define MPID_DEV_WIN_DECL        MPIDI_Devwin_t  dev;
#define MPID_DEV_COMM_DECL       MPIDI_Devcomm_t dev;
#define MPID_DEV_OP_DECL         MPIDI_Devop_t   dev;

typedef struct MPIDI_av_entry {
    union {
    MPIDI_NM_ADDR_DECL} netmod;
#ifdef MPIDI_BUILD_CH4_LOCALITY_INFO
    MPIDI_locality_t is_local;
#endif
} MPIDI_av_entry_t;

typedef struct {
    MPIR_OBJECT_HEADER;
    int size;
    MPIDI_av_entry_t table[0];
} MPIDI_av_table_t;

extern MPIDI_av_table_t **MPIDI_av_table;
extern MPIDI_av_table_t *MPIDI_av_table0;

#define MPIDIU_get_av_table(avtid) (MPIDI_av_table[(avtid)])
#define MPIDIU_get_av(avtid, lpid) (MPIDI_av_table[(avtid)]->table[(lpid)])

#define MPIDIU_get_node_map(avtid)   (MPIDI_CH4_Global.node_map[(avtid)])

#define MPID_Progress_register_hook(fn_, id_) MPID_Progress_register(fn_, id_)
#define MPID_Progress_deregister_hook(id_) MPID_Progress_deregister(id_)
#define MPID_Progress_activate_hook(id_) MPID_Progress_activate(id_)
#define MPID_Progress_deactivate_hook(id_) MPID_Progress_deactivate(id_)

#define HAVE_DEV_COMM_HOOK
#define MPID_Comm_create_hook   MPIDI_Comm_create_hook
#define MPID_Comm_free_hook     MPIDI_Comm_free_hook

MPL_STATIC_INLINE_PREFIX int MPIDI_Type_commit_hook(MPIR_Datatype * type);
MPL_STATIC_INLINE_PREFIX int MPIDI_Type_free_hook(MPIR_Datatype * type);

#define MPID_Type_commit_hook   MPIDI_Type_commit_hook
#define MPID_Type_free_hook     MPIDI_Type_free_hook

#define MPID_Op_commit_hook     MPIDI_Op_commit_hook
#define MPID_Op_free_hook       MPIDI_Op_free_hook

/*
 * operation for (avtid, lpid) to/from "lupid"
 * 1 bit is reserved for "new_avt_mark". It will be cleared before accessing
 * the avtid and lpid. Therefore, the avtid mask does have that bit set to 0
 */
#define MPIDIU_AVTID_BITS                    (7)
#define MPIDIU_LPID_BITS                     (8 * sizeof(int) - (MPIDIU_AVTID_BITS + 1))
#define MPIDIU_LPID_MASK                     (0xFFFFFFFFU >> (MPIDIU_AVTID_BITS + 1))
#define MPIDIU_AVTID_MASK                    (~MPIDIU_LPID_MASK)
#define MPIDIU_NEW_AVT_MARK                  (0x80000000U)
#define MPIDIU_LUPID_CREATE(avtid, lpid)      (((avtid) << MPIDIU_LPID_BITS) | (lpid))
#define MPIDIU_LUPID_GET_AVTID(lupid)          ((((lupid) & MPIDIU_AVTID_MASK) >> MPIDIU_LPID_BITS))
#define MPIDIU_LUPID_GET_LPID(lupid)           (((lupid) & MPIDIU_LPID_MASK))
#define MPIDIU_LUPID_SET_NEW_AVT_MARK(lupid)   ((lupid) |= MPIDIU_NEW_AVT_MARK)
#define MPIDIU_LUPID_CLEAR_NEW_AVT_MARK(lupid) ((lupid) &= (~MPIDIU_NEW_AVT_MARK))
#define MPIDIU_LUPID_IS_NEW_AVT(lupid)         ((lupid) & MPIDIU_NEW_AVT_MARK)

#define MPIDI_DYNPROC_MASK                 (0x80000000U)

#define MPID_INTERCOMM_NO_DYNPROC(comm) \
    (MPIDI_COMM((comm),map).avtid == 0 && MPIDI_COMM((comm),local_map).avtid == 0)

int MPIDI_check_for_failed_procs(void);

#ifdef HAVE_SIGNAL
void MPIDI_sigusr1_handler(int sig);
#endif

#include "mpidu_pre.h"

#endif /* MPIDPRE_H_INCLUDED */
