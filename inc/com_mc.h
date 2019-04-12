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

#ifndef _COM_MC_H_
#define _COM_MC_H_

#ifdef __cplusplus

extern "C"
{
#endif

typedef void (*COM_MC_L) (pel *ref, int gmv_x, int gmv_y, int s_ref, int s_pred, pel *pred, int w, int h, int bit_depth, int bHpFilter);
typedef void (*COM_MC_C) (pel *ref, int gmv_x, int gmv_y, int s_ref, int s_pred, pel *pred, int w, int h, int bit_depth, int bHpFilter);

extern COM_MC_L com_tbl_mc_l[2][2];
extern COM_MC_C com_tbl_mc_c[2][2];

#define com_mc_l_hp(ref, gmv_x, gmv_y, s_ref, s_pred, pred, w, h, bit_depth) \
    (com_tbl_mc_l[(gmv_x & 0xF)?1:0][(gmv_y & 0xF)?1:0])\
        (ref, gmv_x, gmv_y, s_ref, s_pred, pred, w, h, bit_depth, 1)

#define com_mc_l(ref, gmv_x, gmv_y, s_ref, s_pred, pred, w, h, bit_depth) \
    (com_tbl_mc_l[(gmv_x & 0x3)?1:0][(gmv_y & 0x3)?1:0])\
        (ref, gmv_x, gmv_y, s_ref, s_pred, pred, w, h, bit_depth, 0)

#define com_mc_c_hp(ref, gmv_x, gmv_y, s_ref, s_pred, pred, w, h, bit_depth) \
    (com_tbl_mc_c[(gmv_x & 0x1F)?1:0][(gmv_y & 0x1F)?1:0])\
        (ref, gmv_x, gmv_y, s_ref, s_pred, pred, w, h, bit_depth, 1)

#define com_mc_c(ref, gmv_x, gmv_y, s_ref, s_pred, pred, w, h, bit_depth) \
    (com_tbl_mc_c[(gmv_x & 0x7)?1:0][(gmv_y & 0x7)?1:0])\
        (ref, gmv_x, gmv_y, s_ref, s_pred, pred, w, h, bit_depth, 0)


void com_mc(int x, int y, int pic_w, int pic_h, int w, int h, s8 refi[REFP_NUM], s16 mv[REFP_NUM][MV_D], COM_REFP(*refp)[REFP_NUM], pel pred_buf[N_C][MAX_CU_DIM], int pred_stride, CHANNEL_TYPE channel, int bit_depth
#if APPLY_IC
	, pel *pred_forward, pel *pred_back, int *backflag/*, s16 mv[REFP_NUM][MV_D], int x, int y, u16 avail_cu,int icflagg0,int icflagg1*/
#endif // APPLY_IC
);

#if APPLY_IC
int GetMSB(unsigned int x);
void xGetLLSICPrediction(const int compID, const int bitDepth, int L0L1, COM_PIC  *refp, int *as, pel *pRecc, int iRecStride, int cu_width_log2, int cu_height_log2, s16 mv[REFP_NUM][MV_D], int x, int y, u16 avail_cu, s16(*mv_t)[MV_D]);

#endif // APPLY_IC

void mv_clip(int x, int y, int pic_w, int pic_h, int w, int h,
             s8 refi[REFP_NUM], s16 mv[REFP_NUM][MV_D], s16(*mv_t)[MV_D]);

void com_affine_mc(int x, int y, int pic_w, int pic_h, int w, int h, s8 refi[REFP_NUM], CPMV mv[REFP_NUM][VER_NUM][MV_D], COM_REFP(*refp)[REFP_NUM], pel pred[N_C][MAX_CU_DIM], int cp_num, COM_PIC_HEADER * pic_header, int scup, int pic_width_in_scu, s16(*map_mv)[REFP_NUM][MV_D], int bit_depth);
void com_affine_mc_l(int x, int y, int pic_w, int pic_h, int cu_width, int cu_height, CPMV ac_mv[VER_NUM][MV_D], COM_PIC* ref_pic, pel pred[MAX_CU_DIM], int cp_num, int sub_w, int sub_h, int bit_depth);
void com_affine_mc_lc(int x, int y, int pic_w, int pic_h, int cu_width, int cu_height, CPMV ac_mv[VER_NUM][MV_D], COM_PIC* ref_pic, pel pred[N_C][MAX_CU_DIM], int cp_num, int sub_w, int sub_h, int scup, int pic_width_in_scu, s16(*map_mv)[REFP_NUM][MV_D], int lidx, int bit_depth);

#if SIMD_MC
void average_16b_no_clip_sse(s16 *src, s16 *ref, s16 *dst, int s_src, int s_ref, int s_dst, int wd, int ht);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _COM_MC_H_ */
