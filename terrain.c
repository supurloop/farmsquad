/* --------------------------------------------------------------------------------------------- */
/* FarmSquad (terrain.c)                                                                         */
/*                                                                                               */
/* Terrain Generation.                                                                           */
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
#include <string.h>
#include "rand.h"
#include "main.h"
#include "terrain.h"

/* --------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */
void MakeTerrain(void)
{
    tline2 = tline;
#if 0  
    while (!now);

    pterrain2 = terrain + (tline2 * (NUM_COLUMNS - 6)) + 1;
    if (tline != 0)
    {
        memcpy(pterrain2, pterrain2 - (NUM_COLUMNS - 6), (NUM_COLUMNS - 6));
    }

    for (column = 1; column < (NUM_COLUMNS - 6 - 1); column++)
    {
        if (tline2 == 0)
        {
            rfv(r);
            /* Seed with random corn */
            if (r < 16)
            {
                *pterrain2 = 4;
            }
            else if (r < 32)
            {
                *pterrain2 = 3;
            }
            else if (r < 48)
            {
                *pterrain2 = 2;
            }
            else //if (r < 192)
            {
                *pterrain2 = 1;
            }
        }
        else
        {
            // Modify current line in place
            if ((*pterrain2) == 1)
            {
                rfv(r);
                if (r < 64)
                {
                    *(pterrain2 - 1) = 1;
                }
                else if (r < 128)
                {
                    *(pterrain2 - 1) = 0;
                }
                else if (r < 192)
                {
                    *(pterrain2 + 1) = 1;
                }
                else
                {
                    *(pterrain2 + 1) = 0;
                }
            }
            else
            {
                /* Clear existing item */
                *pterrain2 = 0;
            }
        }
        pterrain2++;
    }
#endif

    if (tline2 > 0)
    {
        while (!now);
        pterrain2 = terrain + (tline2 * (NUM_COLUMNS - 6));
        rfv(column);
        column &= 0x1f;
        while (!now);
        pterrain2 += column;
        {
            rfv(r);
            if (r < 16)
            {
                *pterrain2 = 2;
            }
            else if (r < 32)
            {
                *pterrain2 = 3;
            }
            else if (r < 48)
            {
                *pterrain2 = 4 | 0x80;
            }
            pterrain2++;
        }

    }
}

/* --------------------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */
void InitTerrain(void)
{
    //memset(terrain, 3, sizeof(terrain));
    //return;
    //seed = 3;
    for (tline = 0; tline < (TERRAIN_PAGES * NUM_ROWS); tline++)
    {
        pterrain2 = terrain + (tline * (NUM_COLUMNS - 6)) + 1;
        if (tline != 0)
        {
            memcpy(pterrain2, pterrain2 - (NUM_COLUMNS - 6), (NUM_COLUMNS - 6));
        }
        for (column = 1; column < (NUM_COLUMNS - 6 - 1); column++)
        {
            if (tline == 0)
            {
                rfv(r);
                /* Seed with random corn */
                if (r < 32)
                {
                    *pterrain2 = 4;
                }
                else if (r < 64)
                {
                    *pterrain2 = 3;
                }
                else if (r < 128)
                {
                    *pterrain2 = 2;
                }
                else if (r < 192)
                {
                    *pterrain2 = 1;
                }
            }
            else
            {
                // Modify current line in place
                if ((*pterrain2) == 1)
                {
                    rfv(r);
                    if (r < 16)
                    {
                        *(pterrain2 - 1) = 1;
                    }
                    else if (r < 32)
                    {
                        *(pterrain2 - 1) = 0;
                    }
                    else if (r < 192)
                    {
                        *(pterrain2 + 1) = 1;
                    }
                    else
                    {
                        *(pterrain2 + 1) = 0;
                    }
                }
                else
                {
                    /* Clear existing item */
                    *pterrain2 = 0;
                }
            }
            pterrain2++;
        }

        if (tline > 0)
        {
            // Place EMPs
            pterrain2 = terrain + (tline * (NUM_COLUMNS - 6));
            rfv(column);
            column &= 0x1f;
            pterrain2 += column;
            {
                rfv(r);
                if (r < 16)
                {
                    *pterrain2 = 2;
                }
                else if (r < 32)
                {
                    *pterrain2 = 3;
                }
                else if (r < 48)
                {
                    *pterrain2 = 4 | 0x80;
                }
                pterrain2++;
            }
        }
    }
    tline = 0;
}
