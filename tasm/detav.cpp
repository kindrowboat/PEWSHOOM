// $detav.cpp 3.0 milbo$ generate VJ and Rowley mean shapes
// Forden jul 08
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
#include "tasm.hpp"

//-----------------------------------------------------------------------------
// Generate DetAv which is the mean shape of Shapes in the face dector frame.
// Attr specifies which face detector to use, Rowley or Viola Jones
//
// If sShapeFile is not null, we read the face detector shapes saved in it
//
// If sShapeFile is null, we invoke the appropriate detector for each tag,
// which is much slower.  You would do this if you are using shapes that
// do not have the appropriate detector shape in a shape file
//
// This function is based on code by GuoQing Hu a.k.a DrHu, www.drhu.org

static void GenDetAvShape (SHAPE &DetAv,        // out
                unsigned DetAttr,               // in: which detector to use
                const char sShapeFile[],        // in
                bool  fSkipIfNotInShapeFile,    // in:
                const vec_SHAPE Shapes,         // in: ref shapes
                const vec_string &Tags,         // in
                const vec_int &nUsedLandmarks,  // in
                const char sDataDir[])          // in: for face det data files
{
ASSERT(DetAttr & (FA_ViolaJones|FA_Rowley));
ASSERT((DetAttr & ~(FA_ViolaJones|FA_Rowley)) == 0);

const int nPoints = DetAv.nrows();
int nNoEyes = 0, nOneEye = 0, nNoFace = 0;
int nShapes = 0;
const bool fPacify = (Tags.size() > 30);
int nDetShapeFile = 0;

for (unsigned iShape = 0; iShape < Tags.size(); iShape++)
    {
    if (nUsedLandmarks[iShape] == nPoints)  // only use shapes with all points
        {
        // following returns 0 if not found, 1 found by detector, 2 in sShapeFile

        DET_PARAMS DetParams;
        int nGetDet = nGetDetParams(DetParams,
                         sGetBasenameFromTag(Tags[iShape]), // image name
                         DetAttr, sShapeFile, sDataDir,
                         true, fSkipIfNotInShapeFile, false);
        if (nGetDet)
            {
            // found face detector shape

            if (nGetDet == 2)   // track nbr of dets found in shape file
                nDetShapeFile++;

            if (DetParams.lex == INVALID && DetParams.rex == INVALID)
                nNoEyes++;
            else if (DetParams.lex == INVALID || DetParams.rex == INVALID)
                nOneEye++;

            // align the reference shape to this face detector shape

            SHAPE Shape(Shapes[iShape]);

            switch (DetAttr)
                {
                case FA_ViolaJones:
                case FA_Rowley:
                    AlignToDetFrame(Shape, DetParams);
                    break;
                default:
                    Err("GenDetAvShape: bad DetAttr 0x%x", DetAttr);
                }
            DetAv += Shape;
            nShapes++;
            }
        else
            nNoFace++;
        }
    if (iShape == 0)
        {
        // this is here instead of before the loop so it appears after the
        // "Reading muct68.shape" printed by the first call to FindMatInFile

        lprintf("Generating %s mean shape ", sGetAtFaceString(DetAttr));
        if (fPacify)
            InitPacifyUser(Tags.size());
        }
    if (fPacify)
        PacifyUser(iShape);
    }
if (fPacify)
    printf("0 ");   // finish pacifying

// give detailed messages to help debug bad parameters in tasm config file

if (nShapes == 0)
    {
    lprintf("\n");
    if (sShapeFile == NULL)
        Err("found no %s shapes", sGetAtFaceString(DetAttr));
    else if (fSkipIfNotInShapeFile)
        Err("found no %s shapes in %s",
            sGetAtFaceString(DetAttr), sShapeFile);
    else
        Err("found no %s shapes in %s or by invoking the detector",
            sGetAtFaceString(DetAttr), sShapeFile);
    }
lprintf("used %d face det shapes", nShapes);
char *sBaseExt = sGetBaseExt(sShapeFile);
if (nShapes)
    {
    if (nDetShapeFile == nShapes)
        lprintf(", all from %s", sBaseExt);
    else
        lprintf(", %d from %s", nDetShapeFile, sBaseExt);
    if (DetAttr & FA_Rowley)
        lprintf(" (%d no eyes, %d one eye)", nNoEyes, nOneEye);
    }
lprintf("\n");

DetAv /= nShapes;
}

//-----------------------------------------------------------------------------
// initialize VjAv and RowleyAv

void GenDetAvShapes (SHAPE &VjAv,           // out
             SHAPE &RowleyAv,               // out
             const char sShapeFile[],       // in: shape filename e.g. muct68.shape
             bool  fSkipIfNotInShapeFile,   // in:
             const vec_SHAPE Shapes,        // in: ref shapes from file
             const vec_string &Tags,        // in
             const vec_int &nUsedLandmarks, // in
             const char sDataDir[])         // in: for face detector data files
{
GenDetAvShape(VjAv,
              FA_ViolaJones, sShapeFile, fSkipIfNotInShapeFile, Shapes, Tags,
              nUsedLandmarks, sDataDir);

GenDetAvShape(RowleyAv,
              FA_Rowley, sShapeFile, fSkipIfNotInShapeFile, Shapes, Tags,
              nUsedLandmarks, sDataDir);
}
