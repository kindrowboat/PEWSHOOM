// $atface.cpp 3.0 milbo$ face attributes
//-----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// A copy of the GNU General Public License is available at
// http://www.r-project.org/Licenses/
//-----------------------------------------------------------------------------

#include "stasm.hpp"

//-----------------------------------------------------------------------------
char *sGetAtFaceString (unsigned Attr, bool fSpecialHandlingForMask1)
{
static char s[SLEN];
s[0] = 0;
if (!fSpecialHandlingForMask1 || Attr != 0xffffffff)
    {
    unsigned int BitMask = 0x80000000;
    int iAttr = 32;
    while (iAttr--)
        {
        if ((Attr & BitMask) && sgFaceAttr[iAttr][0])
            {
            if (s[0])
                strcat(s, ",");
            strcat(s, sgFaceAttr[iAttr]);
            }
        BitMask >>= 1;
        }
    }
return s;
}

//-----------------------------------------------------------------------------
char *sGetDetString (unsigned Attr)
{
if (Attr & FA_Rowley)
    return (char *)"Rowley";

if (Attr & FA_ViolaJones)
    return (char *)"Viola Jones";

Err("sGetDetString: Illegal Attr %x", Attr);

return NULL;    // keep compiler warnings quiet
}
