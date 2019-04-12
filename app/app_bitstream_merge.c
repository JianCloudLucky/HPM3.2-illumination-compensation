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

#define _CRT_SECURE_NO_WARNINGS


#include <com_typedef.h>
#include "app_util.h"
#include "app_args.h"
#include "com_def.h"
#include "dec_def.h"
#include "dec_DecAdaptiveLoopFilter.h"

#define VERBOSE_FRAME              VERBOSE_1

#define MAX_BS_BUF                 (64*1024*1024) /* byte */

static char op_fname_inp[256] = "\0";
static char op_fname_out[256] = "\0";
static int  op_max_frm_num = 0;
static int  op_use_pic_signature = 0;
static int  op_bit_depth_output = 8;
#if LIBVC_ON
static int  op_merge_libandseq_bitstream_flag = 0;
static char op_fname_inp_lib[256] = "\0";
static char op_fname_inp_seq[256] = "\0";
#endif

typedef enum _STATES
{
    STATE_DECODING,
    STATE_BUMPING
} STATES;

typedef enum _OP_FLAGS
{
    OP_FLAG_FNAME_INP,
    OP_FLAG_FNAME_OUT,
    OP_FLAG_MAX_FRM_NUM,
    OP_FLAG_USE_PIC_SIGN,
    OP_FLAG_OUT_BIT_DEPTH,
#if LIBVC_ON
    OP_FLAG_MERGE_LIBANDSEQ_BITSTREAM,
    OP_FLAG_FNAME_INP_LIB,
    OP_FLAG_FNAME_INP_SEQ,
#endif
    OP_FLAG_VERBOSE,
    OP_FLAG_MAX
} OP_FLAGS;

static int op_flag[OP_FLAG_MAX] = { 0 };

static COM_ARGS_OPTION options[] = \
{
    {
#if LIBVC_ON
        'i', "input", ARGS_TYPE_STRING,
#else
        'i', "input", ARGS_TYPE_STRING | ARGS_TYPE_MANDATORY,
#endif
        &op_flag[OP_FLAG_FNAME_INP], op_fname_inp,
        "file name of input bitstream"
    },
    {
        'o', "output", ARGS_TYPE_STRING,
        &op_flag[OP_FLAG_FNAME_OUT], op_fname_out,
        "file name of decoded output"
    },
    {
        'f', "frames", ARGS_TYPE_INTEGER,
        &op_flag[OP_FLAG_MAX_FRM_NUM], &op_max_frm_num,
        "maximum number of frames to be decoded"
    },
    {
        's', "signature", COM_ARGS_VAL_TYPE_NONE,
        &op_flag[OP_FLAG_USE_PIC_SIGN], &op_use_pic_signature,
        "conformance check using picture signature (HASH)"
    },
    {
        'v', "verbose", ARGS_TYPE_INTEGER,
        &op_flag[OP_FLAG_VERBOSE], &op_verbose,
        "verbose level\n"
        "\t 0: no message\n"
        "\t 1: frame-level messages (default)\n"
        "\t 2: all messages\n"
    },
    {
        COM_ARGS_NO_KEY, "output_bit_depth", ARGS_TYPE_INTEGER,
        &op_flag[OP_FLAG_OUT_BIT_DEPTH], &op_bit_depth_output,
        "output bitdepth (8(default), 10) "
    },
#if LIBVC_ON
    {
        COM_ARGS_NO_KEY, "merge_libandseq_bitstream", ARGS_TYPE_INTEGER,
        &op_flag[OP_FLAG_MERGE_LIBANDSEQ_BITSTREAM], &op_merge_libandseq_bitstream_flag,
        "merge_lib and seq_bitstream flag (0(default), 1) "
    },
    {
        COM_ARGS_NO_KEY, "input_lib_bitstream", ARGS_TYPE_STRING,
        &op_flag[OP_FLAG_FNAME_INP_LIB], &op_fname_inp_lib,
        "file name of input lib bitstream"
    },
    {
        COM_ARGS_NO_KEY, "input_seq_bitstream", ARGS_TYPE_STRING,
        &op_flag[OP_FLAG_FNAME_INP_SEQ], &op_fname_inp_seq,
        "file name of input seq bitstream"
    },
#endif
    { 0, "", COM_ARGS_VAL_TYPE_NONE, NULL, NULL, "" } /* termination */
};

#define NUM_ARG_OPTION   ((int)(sizeof(options)/sizeof(options[0]))-1)
static void print_usage(void)
{
    int i;
    char str[1024];
    v0print("< Usage >\n");
    for (i = 0; i<NUM_ARG_OPTION; i++)
    {
        if (com_args_get_help(options, i, str) < 0) return;
        v0print("%s\n", str);
    }
}

static int xFindNextStartCode(FILE * fp, int * ruiPacketSize, unsigned char *pucBuffer)
{
    unsigned int uiDummy = 0;
    char bEndOfStream = 0;
    size_t ret = 0;
    ret = fread(&uiDummy, 1, 2, fp);
    if (ret != 2)
    {
        return -1;
    }

    if (feof(fp))
    {
        return -1;
    }
    assert(0 == uiDummy);

    ret = fread(&uiDummy, 1, 1, fp);
    if (ret != 1)
    {
        return -1;
    }
    if (feof(fp))
    {
        return -1;
    }
    assert(1 == uiDummy);

    int iNextStartCodeBytes = 0;
    unsigned int iBytesRead = 0;
    unsigned int uiZeros = 0;

    unsigned char pucBuffer_Temp[16];
    int iBytesRead_Temp = 0;
    pucBuffer[iBytesRead++] = 0x00;
    pucBuffer[iBytesRead++] = 0x00;
    pucBuffer[iBytesRead++] = 0x01;
    while (1)
    {
        unsigned char ucByte = 0;
        ret = fread(&ucByte, 1, 1, fp);

        if (feof(fp))
        {
            iNextStartCodeBytes = 0;
            bEndOfStream = 1;
            break;
        }
        pucBuffer[iBytesRead++] = ucByte;
        if (1 < ucByte)
        {
            uiZeros = 0;
        }
        else if (0 == ucByte)
        {
            uiZeros++;
        }
        else if (uiZeros > 1)
        {
            iBytesRead_Temp = 0;
            pucBuffer_Temp[iBytesRead_Temp] = ucByte;

            iBytesRead_Temp++;
            ret = fread(&ucByte, 1, 1, fp);
            if (ret != 1)
            {
                return -1;
            }
            pucBuffer_Temp[iBytesRead_Temp] = ucByte;
            //           iBytesRead = iBytesRead + 1;
            pucBuffer[iBytesRead++] = ucByte;
            iBytesRead_Temp++;
#if LIBVC_ON
            if (pucBuffer_Temp[0] == 0x01 && (pucBuffer_Temp[1] == 0xb3 ||
                                              pucBuffer_Temp[1] == 0xb6 || pucBuffer_Temp[1] == 0xb0 || pucBuffer_Temp[1] == 0xb1
                                              || pucBuffer_Temp[1] == 0x00))
            {
#else
            if (pucBuffer_Temp[0] == 0x01 && (pucBuffer_Temp[1] == 0xb3 ||
                                              pucBuffer_Temp[1] == 0xb6 || pucBuffer_Temp[1] == 0xb0
                                              || pucBuffer_Temp[1] == 0x00))
            {
#endif
                iNextStartCodeBytes = 2 + 1 + 1;

                uiZeros = 0;
                break;
            }
            else
            {
                uiZeros = 0;
                iNextStartCodeBytes = 0;// 2 + 1 + 1;

            }

        }
        else
        {
            uiZeros = 0;
        }
    }

    *ruiPacketSize = iBytesRead - iNextStartCodeBytes;

    if (bEndOfStream)
    {
        return 0;
    }
    if (fseek(fp, -1 * iNextStartCodeBytes, SEEK_CUR) != 0)
    {
        printf("file seek failed!\n");
    };

    return 0;
}


static unsigned int  initParsingConvertPayloadToRBSP(const unsigned int uiBytesRead, unsigned char* pBuffer, unsigned char* pBuffer2)
{
    unsigned int uiZeroCount = 0;
    unsigned int uiBytesReadOffset = 0;
    unsigned int uiBitsReadOffset = 0;
    const unsigned char *pucRead = pBuffer;
    unsigned char *pucWrite = pBuffer2;

    unsigned char ucHeaderType;
    unsigned int uiWriteOffset = uiBytesReadOffset;
    ucHeaderType = pucRead[uiBytesReadOffset++]; //HeaderType



    switch (ucHeaderType)
    {
    case 0xb5:
        if ((pucRead[uiBytesReadOffset] >> 4) == 0x0d)
        {
            break;
        }
        else
        {
        }
    case 0xb0:
    case 0xb2:
        //       initParsing(uiBytesRead);
        memcpy(pBuffer2, pBuffer, uiBytesRead);

        return uiBytesRead;
    default:
        break;
    }
    pucWrite[uiWriteOffset] = ucHeaderType;

    uiWriteOffset++;

    unsigned char ucCurByte = pucRead[uiBytesReadOffset];

    for (; uiBytesReadOffset < uiBytesRead; uiBytesReadOffset++)
    {
        ucCurByte = pucRead[uiBytesReadOffset];

        if (2 <= uiZeroCount && 0x02 == pucRead[uiBytesReadOffset])
        {
            pucWrite[uiWriteOffset] = ((pucRead[uiBytesReadOffset] >> 2) << (uiBitsReadOffset + 2));
            uiBitsReadOffset += 2;
            uiZeroCount = 0;
            //printf("\n error uiBitsReadOffset \n");
            if (uiBitsReadOffset >= 8)
            {
                uiBitsReadOffset = 0;
                continue;
            }
            if (uiBytesReadOffset >= uiBytesRead)
            {
                break;
            }
        }
        else if (2 <= uiZeroCount && 0x01 == pucRead[uiBytesReadOffset])
        {
            uiBitsReadOffset = 0;
            pucWrite[uiWriteOffset] = pucRead[uiBytesReadOffset];
        }
        else
        {
            pucWrite[uiWriteOffset] = (pucRead[uiBytesReadOffset] << uiBitsReadOffset);
        }

        if (uiBytesReadOffset + 1 < uiBytesRead)
        {
            pucWrite[uiWriteOffset] |= (pucRead[uiBytesReadOffset + 1] >> (8 - uiBitsReadOffset));

        }
        uiWriteOffset++;

        if (0x00 == ucCurByte)
        {
            uiZeroCount++;
        }
        else
        {
            uiZeroCount = 0;
        }
    }

    // th just clear the remaining bits in the buffer
    for (unsigned int ui = uiWriteOffset; ui < uiBytesRead; ui++)
    {
        pucWrite[ui] = 0;
    }
    //    initParsing(uiWriteOffset);
    memcpy(pBuffer, pBuffer2, uiWriteOffset);


    return uiBytesRead;
}


static int read_a_bs(FILE * fp, int * pos, unsigned char * bs_buf, unsigned char * bs_buf2)
{
    int read_size, bs_size;

    unsigned char b = 0;
    bs_size = 0;
    read_size = 0;
    if (!fseek(fp, *pos, SEEK_SET))
    {
        int ret = 0;
        ret = xFindNextStartCode(fp, &bs_size, bs_buf);
        if (ret == -1)
        {
            v2print("End of file\n");
            return -1;
        }
#if LIBVC_ON
        read_size = bs_size;
#else
        read_size = initParsingConvertPayloadToRBSP(bs_size, bs_buf, bs_buf2);
#endif

    }
    else
    {
        v0print("Cannot seek bitstream!\n");
        return -1;
    }
    return read_size;
}

int print_stat(DEC_STAT * stat, int ret)
{
    char stype;
    int i, j;
    if (COM_SUCCEEDED(ret))
    {
        if (stat->ctype == COM_CT_PICTURE)
        {
            switch (stat->stype)
            {
            case COM_ST_I:
                stype = 'I';
                break;
            case COM_ST_P:
                stype = 'P';
                break;
            case COM_ST_B:
                stype = 'B';
                break;
            case COM_ST_UNKNOWN:
            default:
                stype = 'U';
                break;
            }
            v1print("%c-slice", stype);
        }
        else if (stat->ctype == COM_CT_SQH)
        {
            v1print("Sequence header");
        }
        else
        {
            v0print("Unknown bitstream");
        }
        v1print(" (read=%d, poc=%d) ", stat->read, (int)stat->poc);
        for (i = 0; i < 2; i++)
        {
            v1print("[L%d ", i);
            for (j = 0; j < stat->refpic_num[i]; j++) v1print("%d ", stat->refpic[i][j]);
            v1print("] ");
        }
        if (ret == COM_OK)
        {
            v1print("\n");
        }
        else if (ret == COM_OK_FRM_DELAYED)
        {
            v1print("->Frame delayed\n");
        }
        else if (ret == COM_OK_DIM_CHANGED)
        {
            v1print("->Resolution changed\n");
        }
        else
        {
            v1print("->Unknown OK code = %d\n", ret);
        }
    }
    else
    {
        v0print("Decoding error = %d\n", ret);
    }
    return 0;
}

static int set_extra_config(DEC id)
{
    int  ret, size, value;
    if (op_use_pic_signature)
    {
        value = 1;
        size = 4;
        ret = dec_config(id, DEC_CFG_SET_USE_PIC_SIGNATURE, &value, &size);
        if (COM_FAILED(ret))
        {
            v0print("failed to set config for picture signature\n");
            return -1;
        }
    }
    return 0;
}

/* convert DEC into DEC_CTX with return value if assert on */
#define DEC_ID_TO_CTX_RV(id, ctx, ret) \
    com_assert_rv((id), (ret)); \
    (ctx) = (DEC_CTX *)id; \
    com_assert_rv((ctx)->magic == DEC_MAGIC_CODE, (ret));

void write_tmp_bs(COM_PIC_HEADER * pic_header, int add_dtr, unsigned char *bs_buf, FILE * merge_fp)
{
    u8 tmp_bs;
    u16 dtr = pic_header->dtr + add_dtr;
    tmp_bs = bs_buf[4] & 0xE0;
    tmp_bs |= (dtr >> 5) & 0x1F;
    fwrite(&tmp_bs, 1, 1, merge_fp);

    tmp_bs = bs_buf[5] & 0x07;
    tmp_bs |= (dtr & 0x1F) << 3;
    fwrite(&tmp_bs, 1, 1, merge_fp);
}

int main(int argc, const char **argv)
{
    unsigned char    * bs_buf = NULL;
    DEC              id = NULL;
    DEC_CDSC         cdsc;
    COM_BITB          bitb;
    /*temporal buffer for video bit depth less than 10bit */
    COM_IMGB        * imgb_t = NULL;
    int                ret;
    int                bs_size, bs_read_pos = 0;
    FILE             * fp_bs = NULL;
    FILE             * fp_bs_write = NULL;
    int                bs_num, max_bs_num;
    u8                 tmp_size[4];
    int                bs_end_pos;
    int                intra_dist[2];
    int                intra_dist_idx = 0;
    COM_BSR         * bs;
    COM_SQH         * sqh;
    COM_PIC_HEADER  * pic_header;
    COM_CNKH        * cnkh;
    DEC_CTX        * ctx;

#if LIBVC_ON
    unsigned char      *bs_buf_lib2 = NULL;
    DEC                id_lib = NULL;
    DEC                id_seq = NULL;
    DEC_CDSC           cdsc_lib;
    DEC_CDSC           cdsc_seq;
    COM_BSR          * bs_lib;
    COM_BSR          * bs_seq;
    COM_SQH          * sqh_lib = NULL;
    COM_SQH          * sqh_seq = NULL;
    COM_PIC_HEADER           * sh_lib = NULL;
    COM_PIC_HEADER           * sh_seq = NULL;
    COM_CNKH         * cnkh_lib;
    COM_CNKH         * cnkh_seq;
    DEC_CTX          * ctx_lib;
    DEC_CTX          * ctx_seq;
    COM_BITB           bitb_lib;
    COM_BITB           bitb_seq;
    FILE             * fp_bs_lib = NULL;
    FILE             * fp_bs_seq = NULL;
    int                bs_end_pos_lib;
    int                bs_end_pos_seq;
    int                bs_read_pos_lib;
    int                bs_read_pos_seq;
    int                bs_read_pos_sqh;
    int                tmp_bs_read_pos_lib;
    unsigned char    * bs_buf_lib_sqh = NULL;
    unsigned char    * bs_buf_seq_sqh = NULL;
    unsigned char    * bs_buf_lib = NULL;
    unsigned char    * bs_buf_seq = NULL;
    u8                 tmp_size_lib_sqh[4];
    u8                 tmp_size_seq_sqh[4];
    u8                 tmp_size_lib[4];
    u8                 tmp_size_seq[4];
    int                bs_size_lib_sqh;
    int                bs_size_seq_sqh;
    int                bs_size_lib;
    int                bs_size_seq;

    LibVCData          libvc_data;
    init_libvcdata(&libvc_data);

    /* parse options */
    ret = com_args_parse_all(argc, argv, options);
    if (ret != 0)
    {
        if (ret > 0) v0print("-%c argument should be set\n", ret);
        if (ret < 0) v0print("Configuration error, please refer to the usage.\n");
        print_usage();
        return -1;
    }

    if (op_merge_libandseq_bitstream_flag)
    {
        //initialize
        int last_referenced_library_index = -1;

        setvbuf(stdout, NULL, _IOLBF, 1024);
        fp_bs_lib = fopen(op_fname_inp_lib, "rb");
        fp_bs_seq = fopen(op_fname_inp_seq, "rb");
        fp_bs_write = fopen(op_fname_out, "wb");
        if (fp_bs_lib == NULL || fp_bs_seq == NULL || fp_bs_write == NULL)
        {
            v0print("ERROR: cannot open input or output bitstream file = %s, %s, %s\n", op_fname_inp_lib, op_fname_inp_seq, op_fname_out);
            print_usage();
            return -1;
        }
        fseek(fp_bs_lib, 0, SEEK_END);
        bs_end_pos_lib = ftell(fp_bs_lib);
        fseek(fp_bs_seq, 0, SEEK_END);
        bs_end_pos_seq = ftell(fp_bs_seq);

        bs_buf_lib = malloc(MAX_BS_BUF);
        bs_buf_seq = malloc(MAX_BS_BUF);
        bs_buf_lib_sqh = malloc(MAX_BS_BUF);
        bs_buf_seq_sqh = malloc(MAX_BS_BUF);
        bs_buf_lib2 = malloc(MAX_BS_BUF);
        if (bs_buf_lib2 == NULL)
        {
            v0print("ERROR: cannot allocate bit buffer, size2=%d\n", MAX_BS_BUF);
            return -1;
        }

        if (bs_buf_lib == NULL || bs_buf_seq == NULL || bs_buf_lib_sqh == NULL || bs_buf_seq_sqh == NULL)
        {
            v0print("ERROR: cannot allocate bit buffer, size=%d\n", MAX_BS_BUF);
            return -1;
        }

        sqh_lib = malloc(sizeof(COM_SQH));
        sqh_seq = malloc(sizeof(COM_SQH));
        sh_lib = malloc(sizeof(COM_PIC_HEADER));
        sh_seq = malloc(sizeof(COM_PIC_HEADER));

        sh_lib->pic_alf_on = (int *)malloc(N_C * sizeof(int));
        sh_lib->m_alfPictureParam = (ALFParam **)malloc((N_C) * sizeof(ALFParam *));
        for (int i = 0; i < N_C; i++)
        {
            sh_lib->m_alfPictureParam[i] = NULL;
            AllocateAlfPar(&(sh_lib->m_alfPictureParam[i]), i);
        }
        sh_seq->pic_alf_on = (int *)malloc(N_C * sizeof(int));
        sh_seq->m_alfPictureParam = (ALFParam **)malloc((N_C) * sizeof(ALFParam *));
        for (int i = 0; i < N_C; i++)
        {
            sh_seq->m_alfPictureParam[i] = NULL;
            AllocateAlfPar(&(sh_seq->m_alfPictureParam[i]), i);
        }

        if (sqh_lib == NULL || sqh_seq == NULL || sh_lib == NULL || sh_seq == NULL || sh_lib->pic_alf_on == NULL || sh_seq->pic_alf_on == NULL || sh_lib->m_alfPictureParam == NULL || sh_seq->m_alfPictureParam == NULL)
        {
            v0print("ERROR: cannot allocate sqh or sh buffer\n");
            return -1;
        }

        id_lib = dec_create(&cdsc_lib, NULL);
        com_scan_tbl_delete();
        id_seq = dec_create(&cdsc_seq, NULL);
        if (id_lib == NULL || id_seq == NULL)
        {
            v0print("ERROR: cannot create decoder\n");
            return -1;
        }
        if (set_extra_config(id_lib) || set_extra_config(id_seq))
        {
            v0print("ERROR: cannot set extra configurations\n");
            return -1;
        }
        DEC_CTX      * tmp_ctx;
        tmp_ctx = (DEC_CTX *)id_lib;
        tmp_ctx->dpm.libvc_data = &libvc_data;
        tmp_ctx = NULL;
        tmp_ctx = (DEC_CTX *)id_seq;
        tmp_ctx->dpm.libvc_data = &libvc_data;
        tmp_ctx = NULL;

        //read lib sqh
        bs_read_pos_lib = 0;
        while (1)
        {
            bs_read_pos_sqh = 0;
            bs_size_lib_sqh = read_a_bs(fp_bs_lib, &bs_read_pos_sqh, bs_buf_lib_sqh, bs_buf_lib2);

            tmp_size_lib_sqh[0] = (bs_size_lib_sqh & 0xff000000) >> 24;
            tmp_size_lib_sqh[1] = (bs_size_lib_sqh & 0x00ff0000) >> 16;
            tmp_size_lib_sqh[2] = (bs_size_lib_sqh & 0x0000ff00) >> 8;
            tmp_size_lib_sqh[3] = (bs_size_lib_sqh & 0x000000ff) >> 0;
            if (bs_size_lib_sqh <= 0)
            {
                v1print("bumping process starting...\n");
                continue;
            }

            bs_read_pos_lib += (bs_size_lib_sqh);
            bitb_lib.addr = bs_buf_lib_sqh;
            bitb_lib.ssize = bs_size_lib_sqh;
            bitb_lib.bsize = MAX_BS_BUF;
            DEC_ID_TO_CTX_RV(id_lib, ctx_lib, COM_ERR_INVALID_ARGUMENT);
            bs_lib = &ctx_lib->bs;
            sqh = &ctx_lib->info.sqh;
            pic_header = &ctx_lib->info.pic_header;
            cnkh_lib = &ctx_lib->info.cnkh;
            /* set error status */
            ctx_lib->bs_err = bitb_lib.err = 0;
            COM_TRACE_SET(1);
            /* bitstream reader initialization */
            com_bsr_init(bs_lib, bitb_lib.addr, bitb_lib.ssize, NULL);
            SET_SBAC_DEC(bs_lib, &ctx_lib->sbac_dec);

            if (bs_lib->cur[3] == 0xB0)
            {
                cnkh_lib->ctype = COM_CT_SQH;
            }
            else if (bs_lib->cur[3] == 0xB3 || bs_lib->cur[3] == 0xB6)
            {
                cnkh_lib->ctype = COM_CT_PICTURE;
            }
            else if (bs_lib->cur[3] >= 0x00 && bs_lib->cur[3] <= 0x8E)
            {
                cnkh_lib->ctype = COM_CT_SLICE;
            }

            if (cnkh_lib->ctype == COM_CT_SQH)
            {
                ret = dec_eco_sqh(bs_lib, sqh_lib);
                com_assert_rv(COM_SUCCEEDED(ret), ret);

                break;

                //fwrite(tmp_size_lib_sqh, 1, 4, fp_bs_write);
                //fwrite(bs_buf_lib_sqh, 1, bs_size_lib_sqh, fp_bs_write);
            }
            else
            {
                v0print("ERROR: The start code of lib bistream is not SPS \n");
                return -1;
            }
        }

        //read seq sqh
        bs_read_pos_seq = 0;
        while (1)
        {
            bs_read_pos_sqh = 0;
            bs_size_seq_sqh = read_a_bs(fp_bs_seq, &bs_read_pos_sqh, bs_buf_seq_sqh, bs_buf_lib2);

            tmp_size_seq_sqh[0] = (bs_size_seq_sqh & 0xff000000) >> 24;
            tmp_size_seq_sqh[1] = (bs_size_seq_sqh & 0x00ff0000) >> 16;
            tmp_size_seq_sqh[2] = (bs_size_seq_sqh & 0x0000ff00) >> 8;
            tmp_size_seq_sqh[3] = (bs_size_seq_sqh & 0x000000ff) >> 0;
            if (bs_size_seq_sqh <= 0)
            {
                v1print("bumping process starting...\n");
                continue;
            }

            bs_read_pos_seq += (bs_size_seq_sqh);
            bitb_seq.addr = bs_buf_seq_sqh;
            bitb_seq.ssize = bs_size_seq_sqh;
            bitb_seq.bsize = MAX_BS_BUF;
            DEC_ID_TO_CTX_RV(id_seq, ctx_seq, COM_ERR_INVALID_ARGUMENT);
            bs_seq = &ctx_seq->bs;
            sqh = &ctx_seq->info.sqh;
            pic_header = &ctx_seq->info.pic_header;
            cnkh_seq = &ctx_seq->info.cnkh;
            /* set error status */
            ctx_seq->bs_err = bitb_seq.err = 0;
            COM_TRACE_SET(1);
            /* bitstream reader initialization */
            com_bsr_init(bs_seq, bitb_seq.addr, bitb_seq.ssize, NULL);
            SET_SBAC_DEC(bs_seq, &ctx_seq->sbac_dec);

            if (bs_seq->cur[3] == 0xB0)
            {
                cnkh_seq->ctype = COM_CT_SQH;
            }
            else if (bs_seq->cur[3] == 0xB3 || bs_seq->cur[3] == 0xB6)
            {
                cnkh_seq->ctype = COM_CT_PICTURE;
            }
            else if (bs_seq->cur[3] >= 0x00 && bs_seq->cur[3] <= 0x8E)
            {
                cnkh_seq->ctype = COM_CT_SLICE;
            }

            if (cnkh_seq->ctype == COM_CT_SQH)
            {
                ret = dec_eco_sqh(bs_seq, sqh_seq);
                com_assert_rv(COM_SUCCEEDED(ret), ret);

                break;

                //fwrite(tmp_size_seq_sqh, 1, 4, fp_bs_write);
                //fwrite(bs_buf_seq_sqh, 1, bs_size_seq_sqh, fp_bs_write);
            }
            else
            {
                v0print("ERROR: The start code of seq bistream is not SPS \n");
                return -1;
            }
        }

        //merge lib and seq stream
        while (1)
        {
            bs_size_seq = read_a_bs(fp_bs_seq, &bs_read_pos_seq, bs_buf_seq, bs_buf_lib2);

            tmp_size_seq[0] = (bs_size_seq & 0xff000000) >> 24;
            tmp_size_seq[1] = (bs_size_seq & 0x00ff0000) >> 16;
            tmp_size_seq[2] = (bs_size_seq & 0x0000ff00) >> 8;
            tmp_size_seq[3] = (bs_size_seq & 0x000000ff) >> 0;
            if (bs_size_seq <= 0)
            {
                v1print("bumping process starting...\n");
                continue;
            }
            bs_read_pos_seq += (bs_size_seq);
            bitb_seq.addr = bs_buf_seq;
            bitb_seq.ssize = bs_size_seq;
            bitb_seq.bsize = MAX_BS_BUF;
            DEC_ID_TO_CTX_RV(id_seq, ctx_seq, COM_ERR_INVALID_ARGUMENT);
            bs_seq = &ctx_seq->bs;
            sqh = &ctx_seq->info.sqh;
            pic_header = &ctx_seq->info.pic_header;
            cnkh_seq = &ctx_seq->info.cnkh;
            /* set error status */
            ctx_seq->bs_err = bitb_seq.err = 0;
            COM_TRACE_SET(1);
            /* bitstream reader initialization */
            com_bsr_init(bs_seq, bitb_seq.addr, bitb_seq.ssize, NULL);
            SET_SBAC_DEC(bs_seq, &ctx_seq->sbac_dec);

            if (bs_seq->cur[3] == 0xB0)
            {
                cnkh_seq->ctype = COM_CT_SQH;
            }
            else if (bs_seq->cur[3] == 0xB3 || bs_seq->cur[3] == 0xB6)
            {
                cnkh_seq->ctype = COM_CT_PICTURE;
            }
            else if (bs_seq->cur[3] >= 0x00 && bs_seq->cur[3] <= 0x8E)
            {
                cnkh_seq->ctype = COM_CT_SLICE;
            }
            else if (bs_seq->cur[3] == 0xB1)
            {
                cnkh_seq->ctype = COM_CT_SLICE;
            }

            if (cnkh_seq->ctype == COM_CT_SQH)
            {
                ret = dec_eco_sqh(bs_seq, sqh);
                com_assert_rv(COM_SUCCEEDED(ret), ret);
                /*
                fwrite(tmp_size, 1, 4, fp_bs_write);
                fwrite(bs_buf, 1, bs_size, fp_bs_write);
                */
            }
            else if (cnkh_seq->ctype == COM_CT_PICTURE)
            {
                /* decode slice header */
                sh_seq->low_delay = sqh_seq->low_delay;
                sh_seq->tool_alf_on = sqh_seq->adaptive_leveling_filter_enable_flag;
#if RPL_ALGIN_WITH_SEPC_1_MINUS_256
                int need_minus_256 = 0;//MX: useless here ? no operation to the PM.
#endif
                ret = dec_eco_pic_header(bs_seq, sh_seq, sqh_seq
#if RPL_ALGIN_WITH_SEPC_1_MINUS_256
                  , &need_minus_256
#endif
                );
                com_assert_rv(COM_SUCCEEDED(ret), ret);

                //find the referenced library_picture_index
                if (sh_seq->is_RLpic_flag && (sh_seq->rpl_l0.ref_pic_active_num>0 || sh_seq->rpl_l1.ref_pic_active_num>0))
                {
                    int referenced_lib_index;
                    if (sh_seq->rpl_l0.ref_pic_active_num > 0)
                    {
                        referenced_lib_index = sh_seq->rpl_l0.ref_pics_ddoi[0];
                        for (int i = 1; i < sh_seq->rpl_l0.ref_pic_active_num; i++)
                        {
                            if (referenced_lib_index != sh_seq->rpl_l0.ref_pics_ddoi[i])
                            {
                                v0print("ERROR: RL_pic can only reference 1 libpic at now \n");
                                return -1;
                            }
                        }
                        for (int i = 0; i < sh_seq->rpl_l1.ref_pic_active_num; i++)
                        {
                            if (referenced_lib_index != sh_seq->rpl_l1.ref_pics_ddoi[i])
                            {
                                v0print("ERROR: RL_pic can only reference 1 libpic at now \n");
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        referenced_lib_index = sh_seq->rpl_l1.ref_pics_ddoi[0];
                        for (int i = 1; i < sh_seq->rpl_l1.ref_pic_active_num; i++)
                        {
                            if (referenced_lib_index != sh_seq->rpl_l1.ref_pics_ddoi[i])
                            {
                                v0print("ERROR: RL_pic can only reference 1 libpic at now \n");
                                return -1;
                            }
                        }
                    }

                    // when it's the first RL_pic || the referenced_library_picture_index is not same to the last_referenced_library_index
                    if (last_referenced_library_index < 0 || last_referenced_library_index != referenced_lib_index)
                    {
                        //find the referenced lib bitstream
                        tmp_bs_read_pos_lib = bs_read_pos_lib;
                        while (1)
                        {
                            bs_size_lib = read_a_bs(fp_bs_lib, &tmp_bs_read_pos_lib, bs_buf_lib, bs_buf_lib2);

                            tmp_size_lib[0] = (bs_size_lib & 0xff000000) >> 24;
                            tmp_size_lib[1] = (bs_size_lib & 0x00ff0000) >> 16;
                            tmp_size_lib[2] = (bs_size_lib & 0x0000ff00) >> 8;
                            tmp_size_lib[3] = (bs_size_lib & 0x000000ff) >> 0;
                            if (bs_size_lib <= 0)
                            {
                                v1print("bumping process starting...\n");
                                continue;
                            }

                            tmp_bs_read_pos_lib += (bs_size_lib);
                            bitb_lib.addr = bs_buf_lib;
                            bitb_lib.ssize = bs_size_lib;
                            bitb_lib.bsize = MAX_BS_BUF;
                            DEC_ID_TO_CTX_RV(id_lib, ctx_lib, COM_ERR_INVALID_ARGUMENT);
                            bs_lib = &ctx_lib->bs;
                            sqh = &ctx_lib->info.sqh;
                            pic_header = &ctx_lib->info.pic_header;
                            cnkh_lib = &ctx_lib->info.cnkh;
                            /* set error status */
                            ctx_lib->bs_err = bitb_lib.err = 0;
                            COM_TRACE_SET(1);
                            /* bitstream reader initialization */
                            com_bsr_init(bs_lib, bitb_lib.addr, bitb_lib.ssize, NULL);
                            SET_SBAC_DEC(bs_lib, &ctx_lib->sbac_dec);

                            if (bs_lib->cur[3] == 0xB0)
                            {
                                cnkh_lib->ctype = COM_CT_SQH;
                            }
                            else if (bs_lib->cur[3] == 0xB3 || bs_lib->cur[3] == 0xB6)
                            {
                                cnkh_lib->ctype = COM_CT_PICTURE;
                            }
                            else if (bs_lib->cur[3] >= 0x00 && bs_lib->cur[3] <= 0x8E)
                            {
                                cnkh_lib->ctype = COM_CT_SLICE;
                            }

                            if (cnkh_lib->ctype == COM_CT_SQH)
                            {
                                ret = dec_eco_sqh(bs_lib, sqh);
                                com_assert_rv(COM_SUCCEEDED(ret), ret);
                                /*
                                fwrite(tmp_size, 1, 4, fp_bs_write);
                                fwrite(bs_buf, 1, bs_size, fp_bs_write);
                                */
                            }
                            else if (cnkh_lib->ctype == COM_CT_PICTURE)
                            {
                                /* decode slice header */
                                sh_lib->low_delay = sqh_lib->low_delay;
                                sh_lib->tool_alf_on = sqh_lib->adaptive_leveling_filter_enable_flag;
#if RPL_ALGIN_WITH_SEPC_1_MINUS_256
                                int need_minus_256 = 0;//MX: useless here ? no operation to the PM.
#endif
                                ret = dec_eco_pic_header(bs_lib, sh_lib, sqh_lib
#if RPL_ALGIN_WITH_SEPC_1_MINUS_256
                                  , &need_minus_256
#endif
                                );
                                com_assert_rv(COM_SUCCEEDED(ret), ret);

                                if (sh_lib->library_picture_index == referenced_lib_index)
                                {
                                    //write lib sps
                                    //fwrite(tmp_size_lib_sqh, 1, 4, fp_bs_write);
                                    fwrite(bs_buf_lib_sqh, 1, bs_size_lib_sqh, fp_bs_write);

                                    //write lib sh
                                    /* chnk size    */
                                    //fwrite(tmp_size_lib, 1, 4, fp_bs_write);
                                    fwrite(bs_buf_lib, 1, bs_size_lib, fp_bs_write);

                                    //set last_referenced_library_index
                                    last_referenced_library_index = referenced_lib_index;
                                }
                            }
                            else if (cnkh_lib->ctype == COM_CT_SLICE)
                            {
                                if (sh_lib->library_picture_index == referenced_lib_index)
                                {
                                    //write lib pic
                                    fwrite(bs_buf_lib, 1, bs_size_lib, fp_bs_write);

                                    //write seq sps
                                    //fwrite(tmp_size_seq_sqh, 1, 4, fp_bs_write);
                                    fwrite(bs_buf_seq_sqh, 1, bs_size_seq_sqh, fp_bs_write);

                                    break;
                                }

                            }

                            if (tmp_bs_read_pos_lib == bs_end_pos_lib)
                            {
                                v0print("ERROR: the referenced lib pic is not in the input lib bitstream! \n");
                                return -1;

                            }
                        }
                    }
                    else
                    {
                        //write seq sps
                        //fwrite(tmp_size_seq_sqh, 1, 4, fp_bs_write);
                        fwrite(bs_buf_seq_sqh, 1, bs_size_seq_sqh, fp_bs_write);
                    }
                }
                else if (sh_seq->slice_type == SLICE_I)
                {
                    //write seq sps
                    //fwrite(tmp_size_seq_sqh, 1, 4, fp_bs_write);
                    fwrite(bs_buf_seq_sqh, 1, bs_size_seq_sqh, fp_bs_write);
                }

                /* chnk size    */
                //fwrite(tmp_size_seq, 1, 4, fp_bs_write);
                fwrite(bs_buf_seq, 1, bs_size_seq, fp_bs_write);
            }
            else if (cnkh_seq->ctype == COM_CT_SLICE)
            {
                fwrite(bs_buf_seq, 1, bs_size_seq, fp_bs_write);
            }

            if (bs_read_pos_seq == bs_end_pos_seq)
            {
                break;
            }
        }

        if (sh_lib->pic_alf_on) free(sh_lib->pic_alf_on);
        if (sh_seq->pic_alf_on) free(sh_seq->pic_alf_on);
        if (sh_lib->m_alfPictureParam)
        {
            for (int g = 0; g < (int)N_C; g++)
            {
                freeAlfPar(sh_lib->m_alfPictureParam[g], g);
            }
            free(sh_lib->m_alfPictureParam);
            sh_lib->m_alfPictureParam = NULL;
        }
        if (sh_seq->m_alfPictureParam)
        {
            for (int g = 0; g < (int)N_C; g++)
            {
                freeAlfPar(sh_seq->m_alfPictureParam[g], g);
            }
            free(sh_seq->m_alfPictureParam);
            sh_seq->m_alfPictureParam = NULL;
        }
        if (imgb_t) imgb_free(imgb_t);
        if (id_lib) dec_delete(id_lib);
        ret = com_scan_tbl_init();
        if (ret != COM_OK)
        {
            v0print("ERROR: can not init scan_tbl\n");
            return -1;
        }
        if (id_seq) dec_delete(id_seq);
        if (fp_bs_lib) fclose(fp_bs_lib);
        if (fp_bs_seq) fclose(fp_bs_seq);
        if (sqh_lib) free(sqh_lib);
        if (sqh_seq) free(sqh_seq);
        if (sh_lib) free(sh_lib);
        if (sh_seq) free(sh_seq);
        if (bs_buf_lib) free(bs_buf_lib);
        if (bs_buf_seq) free(bs_buf_seq);
        if (bs_buf_lib_sqh) free(bs_buf_lib_sqh);
        if (bs_buf_seq_sqh) free(bs_buf_seq_sqh);
        if (fp_bs_write) fclose(fp_bs_write);
        delete_libvcdata(&libvc_data);
        return 0;
    }
    else
#endif
    {
        // set line buffering (_IOLBF) for stdout to prevent incomplete logs when app crashed.
        setvbuf(stdout, NULL, _IOLBF, 1024);
        max_bs_num = argc - 2;
        fp_bs_write = fopen(argv[max_bs_num + 1], "wb");
        for (bs_num = 0; bs_num < max_bs_num; bs_num++)
        {
            fp_bs = fopen(argv[bs_num + 1], "rb");
            if (fp_bs == NULL)
            {
                v0print("ERROR: cannot open bitstream file = %s\n", op_fname_inp);
                print_usage();
                return -1;
            }
            fseek(fp_bs, 0, SEEK_END);
            bs_end_pos = ftell(fp_bs);
            intra_dist_idx = 0;
            if (bs_num) intra_dist[0] += intra_dist[1];
            bs_buf = malloc(MAX_BS_BUF);
            if (bs_buf == NULL)
            {
                v0print("ERROR: cannot allocate bit buffer, size=%d\n", MAX_BS_BUF);
                return -1;
            }
            id = dec_create(&cdsc, NULL);
            if (id == NULL)
            {
                v0print("ERROR: cannot create decoder\n");
                return -1;
            }
            if (set_extra_config(id))
            {
                v0print("ERROR: cannot set extra configurations\n");
                return -1;
            }
            bs_read_pos = 0;
            while (1)
            {
                bs_size = read_a_bs(fp_bs, &bs_read_pos, bs_buf, bs_buf_lib2);

                tmp_size[0] = (bs_size & 0xff000000) >> 24;
                tmp_size[1] = (bs_size & 0x00ff0000) >> 16;
                tmp_size[2] = (bs_size & 0x0000ff00) >> 8;
                tmp_size[3] = (bs_size & 0x000000ff) >> 0;
                if (bs_size <= 0)
                {
                    v1print("bumping process starting...\n");
                    continue;
                }

                bs_read_pos += (bs_size);
                bitb.addr = bs_buf;
                bitb.ssize = bs_size;
                bitb.bsize = MAX_BS_BUF;
                DEC_ID_TO_CTX_RV(id, ctx, COM_ERR_INVALID_ARGUMENT);
                bs = &ctx->bs;
                sqh = &ctx->info.sqh;
                pic_header = &ctx->info.pic_header;
                cnkh = &ctx->info.cnkh;
                /* set error status */
                ctx->bs_err = bitb.err = 0;
                COM_TRACE_SET(1);
                /* bitstream reader initialization */
                com_bsr_init(bs, bitb.addr, bitb.ssize, NULL);
                SET_SBAC_DEC(bs, &ctx->sbac_dec);

                if (bs->cur[3] == 0xB0)
                {
                    cnkh->ctype = COM_CT_SQH;
                }
                else if (bs->cur[3] == 0xB3 || bs->cur[3] == 0xB6)
                {
                    cnkh->ctype = COM_CT_PICTURE;
                }
                else if (bs->cur[3] >= 0x00 && bs->cur[3] <= 0x8E)
                {
                    cnkh->ctype = COM_CT_SLICE;
                }

                if (cnkh->ctype == COM_CT_SQH)
                {
                    ret = dec_eco_sqh(bs, sqh);
                    com_assert_rv(COM_SUCCEEDED(ret), ret);
                    if (!bs_num)
                    {
                        fwrite(tmp_size, 1, 4, fp_bs_write);
                        fwrite(bs_buf, 1, bs_size, fp_bs_write);
                    }
                }
                else if (cnkh->ctype == COM_CT_PICTURE)
                {
                    /* decode slice header */
                    pic_header->low_delay = sqh->low_delay;
#if RPL_ALGIN_WITH_SEPC_1_MINUS_256
                    int need_minus_256 = 0;//MX: useless here ? no operation to the PM.
#endif
                    ret = dec_eco_pic_header(bs, pic_header, sqh
#if RPL_ALGIN_WITH_SEPC_1_MINUS_256
                      , &need_minus_256
#endif
                    );
                    com_assert_rv(COM_SUCCEEDED(ret), ret);

#if LIBVC_ON
                    if (bs_num == 0 && (pic_header->slice_type == SLICE_I || (sqh->library_picture_enable_flag && pic_header->is_RLpic_flag)))
#else
                    if (bs_num == 0 && pic_header->slice_type == SLICE_I)
#endif
                    {
                        intra_dist[intra_dist_idx] = pic_header->dtr;
                        intra_dist_idx++;
                    }
                    if (bs_num == 0)
                    {
                        fwrite(tmp_size, 1, 4, fp_bs_write);
                        fwrite(bs_buf, 1, bs_size, fp_bs_write);
                    }
                    else
                    {
#if LIBVC_ON
                        if (!intra_dist_idx && (pic_header->slice_type == SLICE_I || (sqh->library_picture_enable_flag && pic_header->is_RLpic_flag)))
#else
                        if (!intra_dist_idx && pic_header->slice_type == SLICE_I)
#endif
                        {
                            intra_dist_idx++;
                        }
                        else
                        {
                            /* chnk size    */
                            fwrite(tmp_size, 1, 4, fp_bs_write);
                            /* chnk header  */
                            fwrite(bs_buf, 1, 4, fp_bs_write);
                            /* re-write dtr */
                            write_tmp_bs(pic_header, intra_dist[0], bs_buf, fp_bs_write);
                            /* other stream */
                            fwrite(bs_buf + 6, 1, bs_size - 6, fp_bs_write);
                        }
                    }
                }
                else if (cnkh->ctype == COM_CT_SLICE)
                {
                    fwrite(bs_buf, 1, bs_size, fp_bs_write);
                }

                if (bs_read_pos == bs_end_pos)
                {
                    break;
                }
            }
            if (id) dec_delete(id);
            if (imgb_t) imgb_free(imgb_t);
            if (fp_bs) fclose(fp_bs);
            if (bs_buf) free(bs_buf);
        }
        if (fp_bs_write) fclose(fp_bs_write);
        return 0;
    }
}
