/* ====================================================================================================================

  The copyright in this software is being made available under the License included below.
  This software may be subject to other third party and contributor rights, including patent rights, and no such
  rights are granted under this license.

  Copyright (c) 2018, HUAWEI TECHNOLOGIES CO., LTD. All rights reserved.
  Copyright (c) 2018, SAMSUNG ELECTRONICS CO., LTD. All rights reserved.
  Copyright (c) 2018, PEKING UNIVERSITY SHENZHEN GRADUATE SCHOOL. All rights reserved.
  Copyright (c) 2018, PENGCHENG LABORATORY. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted only for
  the purpose of developing standards within Audio and Video Coding Standard Workgroup of China (AVS) and for testing and
  promoting such standards. The following conditions are required to be met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
      the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
      the following disclaimer in the documentation and/or other materials provided with the distribution.
    * The name of HUAWEI TECHNOLOGIES CO., LTD. or SAMSUNG ELECTRONICS CO., LTD. may not be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

* ====================================================================================================================
*/

#ifndef _COM_H_
#define _COM_H_

#ifdef __cplusplus
extern "C"
{
#endif

//normative bugfix after HPM2.4.1
#define FIX68_CHROMA_QP                    1
#define FIX67_DEQUANT                      1
#define LEVEL_CODING_M4747                 1 //fix binarization for level coding (M4747)

#define FIX_DEC_SEQ_END_CODE               1 //fix the detection of video_sequence_end_code at decoder (previously just skip the code)
#define FIX_DEC_INIT                       1 //fix decoder initialization

#define M4724                              1 //adjust the syntax position

#define RPL_ALGIN_WITH_SEPC_1_MINUS_256    1 // align with spec, when DOI of cur pic less than DOIPrev
#if RPL_ALGIN_WITH_SEPC_1_MINUS_256
#define RPL_FIX_ENCODRER_DELTA_DOI         1 // fix the deltaDOI derive, use the doi between[0,255]
#define RPL_FIX_DECODRER_DELTA_DOI         1 // fix the deltaDOI derive, use the doi between[0,255]
#define RPL_FIX_PIC_OUT_CONDITION          1 // pic output use the true condition(doi, not poi),and not use the equal condition(actually,not equal  now)
#endif
#define RPL_FIX_OUTPUT                     1 // fix the DPB output condition setting, align with the spec
#define RPL_FIX_CONFIG_READ                1 // fix the config read
#define RPL_IDR_CONSTRUCT_LIST             1 // even the IDR frm, also construct the RPL. but it is empty.

#define FIX33_PAD_2M_2N                    1 // padding use 2M & 2N
#define FIX42_DT_SIGNAL                    1 // solve the problem that intra dt information is signaled twice in P/B slice

//end of normative bugfix

#define TRANSFORM_SKIP                     0
#define LOSSLESS_MODE                      0

#define IPCM                               1
#if IPCM
#define IPCM_FORCE                         0 // let ipcm be selected more often
#endif

//high level syntax
#define WRITE_MD5_IN_USER_DATA             1 // write per-frame MD5 into user_data after picture header

#define HLS_RPL                            1
#if HLS_RPL
#define DOI_CYCLE_LENGTH                 256 // the length of the DOI cycle.
#endif

#define PATCH                              1

//partition
#define EQT                                1
//debug
#define SPLIT_DEBUG                        1  // some debug code to check the split
#define PLATFORM_GENERAL_DEBUG             1  // some key check points at encoder for detecting potential bugs
/*  ---------------------------   conformance test related, start   -------------------------------------*/
#define FIXED_SPLIT                        0  // fixed split pattern [must be OFF in formal test] <<<<<<<<<----------NOTE
#if FIXED_SPLIT
#define FS_ALL_COMBINATION                 0  // test split combination of 6 depth
#if FS_ALL_COMBINATION
#define START_QT_DEPTH                     0
#define START_SPLIT_MODE                   0  // 0: not constrain; 1~5: constrain 1 split at start qt depth
#define VERIFY_SPLIT                       0  // print target split combination for each CTU
#define FS_SIMPLE_ORDER                    1  // simple order of split modes (from smallest mode value to the largest)
#endif
#define FS_SAME_SIZE_PER_X_CTU             1  // constrained size changed per X CTUs
#if FS_SAME_SIZE_PER_X_CTU
#define FS_SAME_SIZE_X_VAL                 1  // can be any value larger than 0
#endif
#endif
/*  ---------------------------   conformance test related, end     -------------------------------------*/

//coding mode constraint for region of 64 pixels
#define MODE_CONS                          1  // [this macro cannot be off]
//chroma no split for avoiding 2xN chroma blocks
#define CHROMA_NOT_SPLIT                   1  // [this macro cannot be off]
#define CHROMA_NOT_SPLIT_DEBUG             1  // assertion on checking chroma pred / trans block sizes

//for DT and PBT
#define TB_SPLIT_EXT                       1  // extend the framework to support multiple luma prediction & transform blocks (support PBT and Intra DT)
#if TB_SPLIT_EXT
//fast algorithm (common)
#define TR_SAVE_LOAD                       1  // fast algorithm for PBT
#define TR_EARLY_TERMINATE                 1  // fast algorithm for PBT
#endif
#define DT_PARTITION                       1  // this macro must be 1 (DT can be turn off ONLY by configure)
#if DT_PARTITION
#define DT_SYNTAX                          1  // syntax change to support DT (must be 1)
//DT_INTRA
#define DT_INTRA_DISABLE_IN_B_SLICE        0  // encoder disable intra dt in B slices
#define DT_INTRA_FAST_BY_RD                1  // fast algorithm: early skip based on RD cost comparison
#define DT_SAVE_LOAD                       1  // fast algorithm: save & load best part_size
#define DT_INTRA_BUGFIX                    1  // DT intra RDO bugfix
//DEBUG
#define DT_DEBUG                           1
#endif
#define PRINT_CU                           0
#define PRINT_CU_LEVEL_2                   0
#define PRINT_HMVP_FIFO                    0
//**************** This part needs clean-up later (End)

//intra
#define TSCPM                              1
#define SIMP_TSCPM                         1 // simplify compare logic

//inter
#define BD_AFFINE_AMVR                     1  // M4565 combine Affine and AMVR

#define HMVP_RESET                         1
#define EXCLUDE_AFFINE_IN_HMVP             1

#define EXT_AMVR_HMVP                      1

#define SMVD                               1  // Symmetric MVD mode

#define INTER_CU_CONSTRAINT                1

#define SEP_CONTEXT                        1 // Separate context for useless bin

// filter
#define DEBLOCK_M4647                      1

// others
#define LAST_FLAG_TEST                     1 // M4649

#define USE_RDOQ                           1 // Use RDOQ
#define RDO_DBK                            1 // include DBK changes to luma samples into distortion

#define AFFINE_MVF_VERIFY                  0 // for debugging, clean up later

#define CPMV_BIT_DEPTH                     18
#if CPMV_BIT_DEPTH == 18
typedef int                                CPMV;
#else
typedef short                              CPMV;
#endif
#define COM_CPMV_MAX                       ((s32)((1<<(CPMV_BIT_DEPTH - 1)) - 1))
#define COM_CPMV_MIN                       ((s32)(-(1<<(CPMV_BIT_DEPTH - 1))))

#define LIBVC_ON                           1 // use Library Picture
#if LIBVC_ON
#define LIBVC_BLOCKDISTANCE_BY_LIBPTR      1 // choose the way to process blockdistance between seq_pic and referenced_lib_pic when scale mv
#endif
#define EXTENSION_USER_DATA                1
//fast algorithm
#define ENC_ECU_DEPTH                      4 // for early CU termination
#define ENC_ECU_ADAPTIVE                   1 // for early CU termination
#define MULTI_REF_ME_STEP                  1 // for ME speed-up

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                              SIMD Optimizations                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
#define X86_SSE                            1
#if X86_SSE
#define SIMD_MC                            1
#define SIMD_SAD                           1
#define SIMD_SSD                           1
#define SIMD_DIFF                          1
#define SIMD_HAD_SAD                       1
#define SIMD_AFFINE                        1
#else
#define SIMD_MC                            0
#define SIMD_SAD                           0
#define SIMD_SSD                           0
#define SIMD_DIFF                          0
#define SIMD_HAD_SAD                       0
#define SIMD_AFFINE                        0
#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                         Certain Tools Parameters                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#define INC_QT_DEPTH(qtd, smode)           (smode == SPLIT_QUAD? (qtd + 1) : qtd)
#define INC_BET_DEPTH(betd, smode)         (smode != SPLIT_QUAD? (betd + 1) : betd)

#if EQT
#define MAX_SPLIT_NUM                      6
#define SPLIT_CHECK_NUM                    6
#else
#define MAX_SPLIT_NUM                      4
#define SPLIT_CHECK_NUM                    4
#endif

#define USE_SLICE_DQP                      1

/*****************************************************************************
 * return values and error code
 *****************************************************************************/
/* no more frames, but it is OK */
#define COM_OK_NO_MORE_FRM              (205)
/* progress success, but output is not available temporarily */
#define COM_OK_OUT_NOT_AVAILABLE        (204)
/* frame dimension (width or height) has been changed */
#define COM_OK_DIM_CHANGED              (203)
/* decoding success, but output frame has been delayed */
#define COM_OK_FRM_DELAYED              (202)
#define COM_OK                          (0)
#define END_OF_VIDEO_SEQUENCE           (206)
#define NOT_END_OF_VIDEO_SEQUENCE       (207)

#define COM_ERR                         (-1) /* generic error */
#define COM_ERR_INVALID_ARGUMENT        (-101)
#define COM_ERR_OUT_OF_MEMORY           (-102)
#define COM_ERR_REACHED_MAX             (-103)
#define COM_ERR_UNSUPPORTED             (-104)
#define COM_ERR_UNEXPECTED              (-105)
#define COM_ERR_BAD_CRC                 (-130) /* not matched CRC value */

#define COM_ERR_UNSUPPORTED_COLORSPACE  (-201)
#define COM_ERR_MALFORMED_BITSTREAM     (-202)

#define COM_ERR_UNKNOWN                 (-32767) /* unknown error */


/* return value checking *****************************************************/
#define COM_SUCCEEDED(ret)              ((ret) >= 0)
#define COM_FAILED(ret)                 ((ret) < 0)

/* YUV planar ****************************************************************/
#define COM_COLORSPACE_YUV400          300 /* Y 8bit */
#define COM_COLORSPACE_YUV420          301 /* YUV420 8bit */
#define COM_COLORSPACE_YUV422          302 /* YUV422 8bit narrow chroma*/
#define COM_COLORSPACE_YUV444          303 /* YUV444 8bit */
#define COM_COLORSPACE_YUV422N         COM_COLORSPACE_YUV422
#define COM_COLORSPACE_YUV422W         310 /* YUV422 8bit wide chroma */

#define COM_COLORSPACE_YUV400A8        400 /* Y+alpha 8bit */
#define COM_COLORSPACE_YUV420A8        401 /* YUV420+alpha 8bit */
#define COM_COLORSPACE_YUV422A8        402 /* YUV422+alpha 8bit narrow chroma*/
#define COM_COLORSPACE_YUV444A8        403 /* YUV444+alpha 8bit */
#define COM_COLORSPACE_YUV422NA8       COM_COLORSPACE_YUV422A8
#define COM_COLORSPACE_YUV422WA8       414 /* YUV422+alpha 8bit wide chroma*/

/* RGB pack ******************************************************************/

/* RGB pack 8bit */
#define COM_COLORSPACE_RGB888          2200
#define COM_COLORSPACE_BGR888          2201

#define COM_COLORSPACE_RGBA8888        2220
#define COM_COLORSPACE_BGRA8888        2221
#define COM_COLORSPACE_ARGB8888        2222
#define COM_COLORSPACE_ABGR8888        2223

/*****************************************************************************
 * config types for decoder
 *****************************************************************************/
#define DEC_CFG_SET_USE_PIC_SIGNATURE  (301)

/*****************************************************************************
 * chunk type
 *****************************************************************************/
#define COM_CT_UNKNOWN                  (0)
#define COM_CT_PICTURE                  (1) /* picture header */
#define COM_CT_SQH                      (2) /* sequence header */
#define COM_CT_SLICE                    (3) /* slice header */
#define COM_CT_SIGN                     (6) /* picture signature */
#if FIX_DEC_SEQ_END_CODE
#define COM_CT_SEQ_END                  (7)
#endif

/*****************************************************************************
 * slice type
 *****************************************************************************/
#define COM_ST_UNKNOWN                  (0)
#define COM_ST_I                        (1)
#define COM_ST_P                        (2)
#define COM_ST_B                        (3)

/*****************************************************************************
 * software version
 *****************************************************************************/
#define COM_VER_1                       (1)

/*****************************************************************************
 * type and macro for media time
 *****************************************************************************/
/* media time in 100-nanosec unit */
typedef long long                    COM_MTIME;

/*****************************************************************************
 * image buffer format
 *****************************************************************************
 baddr
    +---------------------------------------------------+ ---
    |                                                   |  ^
    |                                              |    |  |
    |    a                                         v    |  |
    |   --- +-----------------------------------+ ---   |  |
    |    ^  |  (x, y)                           |  y    |  |
    |    |  |   +---------------------------+   + ---   |  |
    |    |  |   |                           |   |  ^    |  |
    |    |  |   |                           |   |  |    |  |
    |    |  |   |                           |   |  |    |  |
    |    |  |   |                           |   |  |    |  |
    |       |   |                           |   |       |
    |    ah |   |                           |   |  h    |  e
    |       |   |                           |   |       |
    |    |  |   |                           |   |  |    |  |
    |    |  |   |                           |   |  |    |  |
    |    |  |   |                           |   |  v    |  |
    |    |  |   +---------------------------+   | ---   |  |
    |    v  |                                   |       |  |
    |   --- +---+-------------------------------+       |  |
    |     ->| x |<----------- w ----------->|           |  |
    |       |<--------------- aw -------------->|       |  |
    |                                                   |  v
    +---------------------------------------------------+ ---

    |<---------------------- stride-------------------->|

 *****************************************************************************/

#define COM_IMGB_MAX_PLANE              (4)

typedef struct _COM_IMGB COM_IMGB;
struct _COM_IMGB
{
    int                 cs; /* color space */
    int                 np; /* number of plane */
    /* width (in unit of pixel) */
    int                 width[COM_IMGB_MAX_PLANE];
    /* height (in unit of pixel) */
    int                 height[COM_IMGB_MAX_PLANE];
    /* buffer stride (in unit of byte) */
    int                 stride[COM_IMGB_MAX_PLANE];
    /* address of each plane */
    void              * addr_plane[COM_IMGB_MAX_PLANE];

    /* time-stamps */
    COM_MTIME          ts[4];

    /* aligned width (in unit of pixel) */
    int                 width_aligned[COM_IMGB_MAX_PLANE];
    /* aligned height (in unit of pixel) */
    int                 height_aligned[COM_IMGB_MAX_PLANE];

    /* left padding size (in unit of pixel) */
    int                 pad_left[COM_IMGB_MAX_PLANE];
    /* right padding size (in unit of pixel) */
    int                 pad_right[COM_IMGB_MAX_PLANE];
    /* up padding size (in unit of pixel) */
    int                 pad_up[COM_IMGB_MAX_PLANE];
    /* bottom padding size (in unit of pixel) */
    int                 pad_down[COM_IMGB_MAX_PLANE];

    /* address of actual allocated buffer */
    void              * buf_addr[COM_IMGB_MAX_PLANE];
    /* actual allocated buffer size */
    int                 buf_size[COM_IMGB_MAX_PLANE];

    /* life cycle management */
    int                 refcnt;
    int                 (*addref)(COM_IMGB * imgb);
    int                 (*getref)(COM_IMGB * imgb);
    int                 (*release)(COM_IMGB * imgb);
};


/*****************************************************************************
 * Bitstream buffer
 *****************************************************************************/
typedef struct _COM_BITB
{
    /* user space address indicating buffer */
    void              * addr;
    void              * addr2;
    /* physical address indicating buffer, if any */
    void              * pddr;
    /* byte size of buffer memory */
    int                 bsize;
    /* byte size of bitstream in buffer */
    int                 ssize;
    /* bitstream has an error? */
    int                 err;
    /* arbitrary data, if needs */
    int                 ndata[4];
    /* arbitrary address, if needs */
    void              * pdata[4];
    /* time-stamps */
    COM_MTIME          ts[4];

} COM_BITB;

/*****************************************************************************
 * description for creating of decoder
 *****************************************************************************/
typedef struct _DEC_CDSC
{
    int            __na; /* nothing */
} DEC_CDSC;

/*****************************************************************************
 * status after decoder operation
 *****************************************************************************/
typedef struct _DEC_STAT
{
    /* byte size of decoded bitstream (read size of bitstream) */
    int            read;
    /* chunk type */
    int            ctype;
    /* slice type */
    int            stype;
    /* frame number monotonically increased whenever decoding a frame.
    note that it has negative value if the decoded data is not frame */
    int            fnum;
    /* picture order count */
    int            poc;
    /* number of reference pictures */
    int            refpic_num[2];
    /* list of reference pictures */
    int            refpic[2][16];
#if LIBVC_ON
    /* is only ref libpic */
    int            is_RLpic_flag;
#endif
} DEC_STAT;

/*****************************************************************************
 * status after encoder operation
 *****************************************************************************/
typedef struct _ENC_STAT
{
    /* encoded bitstream byte size */
    int            write;
    /* picture number increased whenever encoding a frame */
    unsigned long  fnum;
    /* chunk type */
    int            ctype;
    /* slice type */
    int            stype;
    /* quantization parameter used for encoding */
    int            qp;
    /* picture order count */
    int            poc;
    /* number of reference pictures */
    int            refpic_num[2];
    /* list of reference pictures */
    int            refpic[2][16];
#if LIBVC_ON
    /* is only ref libpic */
    int            is_RLpic_flag;
#endif
} ENC_STAT;

#define MAX_NUM_REF_PICS                   17
#define MAX_NUM_ACTIVE_REF_FRAME           4
#define MAX_NUM_RPLS                       32

int g_CountDOICyCleTime;                    // number to count the DOI cycle time.
int g_DOIPrev;                              // the doi of previous frm.


#if HLS_RPL
/* rpl structure */
typedef struct _COM_RPL
{
    int slice_type;
    int poc;
    int tid;
    int ref_pic_num;
    int ref_pic_active_num;
    int ref_pics[MAX_NUM_REF_PICS];
    int ref_pics_ddoi[MAX_NUM_REF_PICS];
#if LIBVC_ON
    int reference_to_library_enable_flag;
    int library_index_flag[MAX_NUM_REF_PICS];
#endif
} COM_RPL;
#endif
/*****************************************************************************
 * API for decoder only
 *****************************************************************************/
/* instance identifier for decoder */
typedef void  * DEC;

DEC dec_create(DEC_CDSC * cdsc, int * err);
void dec_delete(DEC id);
int dec_decode(DEC id, COM_BITB * bitb, DEC_STAT * stat);
int dec_config(DEC id, int cfg, void * buf, int * size);

#ifdef __cplusplus
}
#endif

#endif /* _COM_H_ */
