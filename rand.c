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
#include <stdint.h>

const uint8_t randSeq[256] =
{
    168,  207,  204,  123,  230,  144,   11,  223,  255,  196,  218,  253,  155,   62,  206,   73,
    231,  135,  129,   51,   74,   26,   65,  243,  163,  249,  211,   56,  148,  117,   30,  140,
     15,  242,  157,   36,  197,   50,    0,  208,  210,  125,   53,  162,  234,   52,    8,  175,
     75,   94,   38,  227,  194,  171,  209,   41,  113,  142,  134,   27,   77,  252,  128,  131,
     19,  250,   21,   44,  160,   97,  240,   84,  184,  232,   33,   82,    5,  187,  217,   67,
    222,  105,  225,  199,  169,   17,  215,  228,  190,  254,  176,  104,  177,   99,  114,   22,
     83,   91,  153,   79,  186,  132,  173,  205,  185,  213,  192,  120,  165,   35,  126,  145,
     70,  166,  201,   85,   95,  146,  188,  189,  110,   92,   59,  195,   34,   96,   20,   24,
    245,   89,    3,  127,   42,  203,  159,  115,  156,   16,   45,   58,   25,  229,   39,  152,
    248,  164,  181,   14,   37,  154,  182,  103,  172,  239,  167,  108,  106,   80,  102,   68,
    180,    4,  158,  224,  139,  122,  183,   78,  137,   55,   32,  178,  191,  212,  216,  233,
    116,   72,   60,   90,    7,  251,    2,  109,   66,  100,   13,  200,  141,   29,   93,  219,
     57,  121,   28,   49,  202,   43,   87,  220,  107,   61,   81,  149,  238,   23,   86,   46,
     48,  138,  236,  221,  226,  136,   76,  241,  124,  143,  247,  246,   88,   31,  150,    9,
     64,  119,  112,  193,    6,   10,  235,   47,  174,  130,  111,   40,   71,   12,  244,    1,
     18,  179,  133,  161,   54,  118,  214,  151,   69,   98,  237,   63,  170,  101,  147,  198
};
uint8_t ri;
uint8_t rs;
#define rf() (randSeq[ri++] ^ rs)
#define rfv(r) (r = rf())
