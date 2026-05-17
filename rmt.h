/* --------------------------------------------------------------------------------------------- */
/* FarmSquad (rmt.h)                                                                             */
/*                                                                                               */
/* RMT data and engine.                                                                          */
/*                                                                                               */
/* BSD-3-Clause License                                                                          */
/* Copyright 2024 Supurloop Software LLC                                                         */
/*                                                                                               */
/* Redistribution and use in source and binary forms, with or without modification, are          */
/* permitted provided that the following conditions are met:                                     */
/*                                                                                               */
/* 1. Redistributions of source code must retain the above copyright notice, this list of        */
/* conditions and the following disclaimer.                                                      */
/* 2. Redistributions in binary form must reproduce the above copyright notice, this list of     */
/* conditions and the following disclaimer in the documentation and/or other materials provided  */
/* with the distribution.                                                                        */
/* 3. Neither the name of the copyright holder nor the names of its contributors may be used to  */
/* endorse or promote products derived from this software without specific prior written         */
/* permission.                                                                                   */
/*                                                                                               */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   */
/* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    */
/* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL      */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE */
/* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    */
/* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  */
/* OF THE POSSIBILITY OF SUCH DAMAGE.                                                            */
/* --------------------------------------------------------------------------------------------- */
#ifndef RMT_H_INCLUDE
#define RMT_H_INCLUDE

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define RMTPlayer 0xA500	// location of the player in memory
#define RMTSongRun song_rmt_run
#define RMTSongIdle song_rmt_idle
#define RMTSongEMP song_rmt_emp
//#define RMTSongNotPlaying song_rmt_not_started

#define RMTInitRun \
	__asm__("lda #0");	\
    __asm__("ldx #(<%v)", RMTSongRun);	\
    __asm__("ldy #(>%v)", RMTSongRun);	\
    __asm__("jsr %w", RMTPlayer);

#define RMTInitIdle	\
    __asm__("lda #0");	\
    __asm__("ldx #(<%v)", RMTSongIdle);	\
    __asm__("ldy #(>%v)", RMTSongIdle);	\
    __asm__("jsr %w", RMTPlayer);

#define RMTInitEMP	\
    __asm__("lda #0");	\
    __asm__("ldx #(<%v)", RMTSongEMP);	\
    __asm__("ldy #(>%v)", RMTSongEMP);	\
    __asm__("jsr %w", RMTPlayer);

#define RMTPlay  __asm__("jsr %w+3", RMTPlayer);
#define RMTStop  __asm__("jsr %w+9", RMTPlayer);

extern unsigned char song_rmt_run[];
extern unsigned char song_rmt_idle[];
extern unsigned char song_rmt_emp[];
extern unsigned char RMTPLAYER_rmtplayr_obx[];

#ifdef __cplusplus
}
#endif

#endif /* RMT_H_INCLUDE */
