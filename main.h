/* --------------------------------------------------------------------------------------------- */
/* FarmSquad (main.h)                                                                            */
/*                                                                                               */
/* Atari 8-bit XL/XE arcade game, built using the cc65-2.19 toolchain.                           */
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
#ifndef MAIN_H_INCLUDE
#define MAIN_H_INCLUDE

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define TERRAIN_PAGES (1u)
#define NUM_COLUMNS 40
#define NUM_ROWS 30
#define ROW_ADDR_OFF (3u)
#define NUM_PLAY_COLUMNS (NUM_COLUMNS - (ROW_ADDR_OFF << 1))

typedef enum
{
    /* Main playfield characters */
    CHAR_BLANK = 0,    
    CHAR_CROP = 1,
    CHAR_ROCK = 2,
    CHAR_REPAIR = 3, /* Animated */
    CHAR_EMP = 4, /* Animated */

    /* Border wall characters */
    CHAR_WALL_1 = 5,
    CHAR_WALL_2 = 6,
    CHAR_WALL_3 = 7,
    CHAR_WALL_4 = 8,

    /* Block lettering for FarmSquad graphic */
    CHAR_BLOCK = 9,
    CHAR_LEFT_HALF = 10,
    CHAR_RIGHT_HALF = 11,
    CHAR_CORNER_TOP_LEFT = 12,
    CHAR_CORNER_BOT_LEFT = 13,
    CHAR_CORNER_TOP_RIGHT = 14,
    CHAR_CORNER_BOT_RIGHT = 15,
    CHAR_3_4TH_TOP_LEFT = 16,
    CHAR_3_4TH_BOT_LEFT = 17,
    CHAR_3_4TH_TOP_RIGHT = 18,
    CHAR_3_4TH_BOT_RIGHT = 19,
#if 1
    /* Double Chars x & x + 1*/
    /* 0-9 */
    CHAR_0 = 20,
    CHAR_1 = 22,
    CHAR_2 = 24,
    CHAR_3 = 26,
    CHAR_4 = 28,
    CHAR_5 = 30,
    CHAR_6 = 32,
    CHAR_7 = 34,
    CHAR_8 = 36,
    CHAR_9 = 38,

    /* A-Z */
    CHAR_A = 40,
    CHAR_B = 42,
    CHAR_C = 44,
    CHAR_D = 46,
    CHAR_E = 48,
    CHAR_F = 50,
    CHAR_G = 52,
    CHAR_H = 54,
    CHAR_I = 56,
    CHAR_J = 58,
    CHAR_K = 60,
    CHAR_L = 62,
    CHAR_M = 64,
    CHAR_N = 66,
    CHAR_O = 68,
    CHAR_P = 70,
    CHAR_Q = 72,
    CHAR_R = 74,
    CHAR_S = 76,
    CHAR_T = 78,
    CHAR_U = 80,
    CHAR_V = 82,
    CHAR_W = 84,
    CHAR_X = 86,
    CHAR_Y = 88,
    CHAR_Z = 90,
    CHAR_Z_END = 91,
#endif    
    CHAR_DASH,
    CHAR_DASH_END,
    CHAR_MAX,
} SSFHexCase_t;

extern uint8_t rows[NUM_ROWS][NUM_COLUMNS];
extern uint8_t notice[NUM_COLUMNS];

/* Terrain */
extern uint8_t column;
extern uint8_t r;
extern uint8_t *pterrain2;
extern uint8_t tline2;
extern uint8_t tline;
extern uint8_t now;
extern uint8_t terrain[TERRAIN_PAGES * NUM_ROWS *(NUM_COLUMNS - (ROW_ADDR_OFF << 1))];

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H_INCLUDE */
