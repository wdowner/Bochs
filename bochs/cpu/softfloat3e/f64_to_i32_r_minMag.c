/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016 The Regents of the University of
California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions, and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions, and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 3. Neither the name of the University nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include <stdbool.h>
#include <stdint.h>
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"

int32_t f64_to_i32_r_minMag(float64 a, bool exact, bool saturate, struct softfloat_status_t *status)
{
    int16_t exp;
    uint64_t sig;
    int16_t shiftDist;
    bool sign;
    int32_t absZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    exp = expF64UI(a);
    sig = fracF64UI(a);
    if (softfloat_denormalsAreZeros(status)) {
        if (!exp && sig) return 0;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    shiftDist = 0x433 - exp;
    if (53 <= shiftDist) {
        if (exact && (exp | sig)) {
            softfloat_raiseFlags(status, softfloat_flag_inexact);
        }
        return 0;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    sign = signF64UI(a);
    if (shiftDist < 22) {
        if (sign && (exp == 0x41E) && (sig < UINT64_C(0x0000000000200000))) {
            if (exact && sig) {
                softfloat_raiseFlags(status, softfloat_flag_inexact);
            }
            return -0x7FFFFFFF - 1;
        }

        const int32_t NaN_response = saturate ? 0 : i32_fromNaN; 
        const int32_t NegOverflowResponse = saturate ? i32_minNegativeValue : i32_fromNegOverflow;
        const int32_t PosOverflowResponse = saturate ? i32_maxPositiveValue : i32_fromPosOverflow;

        softfloat_raiseFlags(status, softfloat_flag_invalid);
        return (exp == 0x7FF) && sig 
            ? NaN_response
            : sign ? NegOverflowResponse : PosOverflowResponse;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    sig |= UINT64_C(0x0010000000000000);
    absZ = sig>>shiftDist;
    if (exact && ((uint64_t) (uint32_t) absZ<<shiftDist != sig)) {
        softfloat_raiseFlags(status, softfloat_flag_inexact);
    }
    return sign ? -absZ : absZ;
}

