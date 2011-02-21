/*
 * This file is part of MPlayer CE.
 *
 * MPlayer CE is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * MPlayer CE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with MPlayer CE; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavcodec/dsputil.h"
#include "dsputil_paired.h"
#include "libavutil/ppc/paired.h"

static void vc1_inv_trans_8x8_dc_paired(uint8_t *dest, int linesize, DCTELEM *block)
{
	const float half = 0.5;
	const float scale = 0.140625;
	vector float pair, offset;
	
	offset = psq_l(0,block,1,7);
	asm("fmadds %0,%0,%1,%2" : "+f"(offset) : "f"(scale), "f"(half));
	
	dest -= linesize;
	
	for (int i=0; i<8; i++) {
		pair = psq_lux(dest,linesize,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,0,dest,0,4);
		
		pair = psq_l(2,dest,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,2,dest,0,4);
		
		pair = psq_l(4,dest,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,4,dest,0,4);
		
		pair = psq_l(6,dest,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,6,dest,0,4);
	}
}

static void vc1_inv_trans_8x4_dc_paired(uint8_t *dest, int linesize, DCTELEM *block)
{
	const float half = 0.5;
	const vector float scale = {1.5,0.1328125};
	vector float pair, offset;
	
	offset = psq_l(0,block,1,7);
	offset = paired_merge00(offset, offset);
	offset = ps_madds0(offset, scale, half);
	offset = ps_madds1(offset, scale, half);
	
	dest -= linesize;
	
	for (int i=0; i<4; i++) {
		pair = psq_lux(dest,linesize,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,0,dest,0,4);
		
		pair = psq_l(2,dest,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,2,dest,0,4);
		
		pair = psq_l(4,dest,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,4,dest,0,4);
		
		pair = psq_l(6,dest,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,6,dest,0,4);
	}
}

static void vc1_inv_trans_4x8_dc_paired(uint8_t *dest, int linesize, DCTELEM *block)
{
	const float half = 0.5;
	const vector float scale = {2.125,0.09375};
	vector float pair, offset;
	
	offset = psq_l(0,block,1,7);
	offset = paired_merge00(offset, offset);
	offset = ps_madds0(offset, scale, half);
	offset = ps_madds1(offset, scale, half);
	
	dest -= linesize;
	
	for (int i=0; i<8; i++) {
		pair = psq_lux(dest,linesize,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,0,dest,0,4);
		
		pair = psq_l(2,dest,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,2,dest,0,4);
	}
}

static void vc1_inv_trans_4x4_dc_paired(uint8_t *dest, int linesize, DCTELEM *block)
{
	const float half = 0.5;
	const vector float scale = {2.125,0.1328125};
	vector float pair, offset;
	
	offset = psq_l(0,block,1,7);
	offset = paired_merge00(offset, offset);
	offset = ps_madds0(offset, scale, half);
	offset = ps_madds1(offset, scale, half);
	
	dest -= linesize;
	
	for (int i=0; i<4; i++) {
		pair = psq_lux(dest,linesize,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,0,dest,0,4);
		
		pair = psq_l(2,dest,0,4);
		pair = paired_add(pair, offset);
		psq_st(pair,2,dest,0,4);
	}
}

void vc1dsp_init_paired(DSPContext *dsp, AVCodecContext *avctx)
{
#if 0
	dsp->vc1_inv_trans_8x8_dc = vc1_inv_trans_8x8_dc_paired;
	dsp->vc1_inv_trans_8x4_dc = vc1_inv_trans_8x4_dc_paired;
	dsp->vc1_inv_trans_4x8_dc = vc1_inv_trans_4x8_dc_paired;
	dsp->vc1_inv_trans_4x4_dc = vc1_inv_trans_4x4_dc_paired;
#endif
}
