// $rowley.cpp 3.0 milbo$ interface between ASM software and Rowley face detector
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
// Returns true with face and eye positions in DetParasm if succesful.
// Else returns false with DetParasm untouched.

bool
fRowleyFindFace (DET_PARAMS &DetParams,     // out
                 Image &Img,                // in
                 const char sImage[],       // in
                 const char sDataDir[])     // in
{
DET_PARAMS *pDetParams;
int nFaces = Track_FindAllFaces(&pDetParams, Img, sDataDir);
int iBest = -1; double MaxWidth = 0;
if (nFaces > 0)
    {
    // find biggest face with both eyes

    for (int iFace = 0; iFace < nFaces; iFace++)
        {
        DET_PARAMS *pDet = pDetParams + iFace;
        if (pDet->lex != INVALID && pDet->rex != INVALID && pDet->width > MaxWidth)
            {
            iBest = iFace;
            MaxWidth = pDet->width;
            }
        }
    if (iBest == -1)    // no faces with both eyes?
        {
        // find biggest face with one eye

        for (int iFace = 0; iFace < nFaces; iFace++)
            {
            DET_PARAMS *pDet = pDetParams + iFace;
            if ((pDet->lex != INVALID || pDet->rex != INVALID) && pDet->width > MaxWidth)
                {
                iBest = iFace;
                MaxWidth = pDet->width;
                }
            }
        }
    if (iBest == -1)    // no faces with one eye?
        {
        // find biggest face

        for (int iFace = 0; iFace < nFaces; iFace++)
            {
            DET_PARAMS *pDet = pDetParams + iFace;
            if (pDet->width > MaxWidth)
                {
                iBest = iFace;
                MaxWidth = pDet->width;
                }
            }
        }
    ASSERT(iBest >= 0);
    DetParams = pDetParams[iBest];  // structure assignment
    free(pDetParams);
    }
return iBest >= 0;
}
