// $eyesynth.cpp 3.0 milbo$ synthesize right eye points from left eye points
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
// Synthesize additional points for one eye from the opposite eye
// Return true if did this, else false

static bool fSynthEye (SHAPE &Shape) // io
{
if (Shape.nrows() != 84)
    Err("cannot synthesize eye points from a %d point shape", Shape.nrows());

const int nBasicEyePoints = MLEye - MLEyeOuter;
const int nExtendedEyePoints = 1 + MLEye7 - MLEye0;
ASSERT(nExtendedEyePoints == 8);
bool fSynth = false;

SHAPE LEye(Shape.submat(MLEyeOuter, 0, nBasicEyePoints, 2));
SHAPE REye(Shape.submat(MREyeOuter, 0, nBasicEyePoints, 2));

if (fPointUsed(Shape, MLEye0) && !fPointUsed(Shape, MREye0))
    {
    // Synthesize additional points for the right eye from the extra left eye points
    // NewShape is Shape aligned so left eye is where right eye was

    fSynth = true;
    SHAPE NewShape(TransformShape(Shape, GetAlignTransform(LEye, REye)));

    for (int iPoint = MREye0; iPoint <= MREye7; iPoint++)
        Shape.row(iPoint) = NewShape.row(iPoint - nExtendedEyePoints);
    }
else if (fPointUsed(Shape, MREye0) && !fPointUsed(Shape, MLEye0))
    {
    // Synthesize additional points for the left eye from the extra right eye points
    // Get here for mirrored shapes

    fSynth = true;
    SHAPE NewShape(TransformShape(Shape, GetAlignTransform(REye, LEye)));

    for (int iPoint = MLEye0; iPoint <= MLEye7; iPoint++)
        Shape.row(iPoint) = NewShape.row(iPoint + nExtendedEyePoints);
    }
else if (!fPointUsed(Shape, MREye0) && !fPointUsed(Shape, MLEye0))
    ;
else
    {
    Shape.print("\nBad shape\n");
    Err("SynthREye bad shape");
    }
return fSynth;
}

//-----------------------------------------------------------------------------
void SynthEyePoints (vec_SHAPE &Shapes)
{
lprintf("Synthesizing 8 eye landmarks for a total of 84 landmarks, ");

unsigned nSynth = 0;
for (unsigned iShape = 0; iShape < Shapes.size(); iShape++)
    if (fSynthEye(Shapes[iShape]))
        nSynth++;

lprintf("synthesized %d eyes\n", nSynth);
}

//-----------------------------------------------------------------------------
void InitExtraEyeLandsUsed (vec_bool  &fExtraLEyeLandsUsed,  // out
                            vec_bool  &fExtraREyeLandsUsed,  // out
                            vec_SHAPE &OrgShapes)            // in
{
const int nShapes = OrgShapes.size();
fExtraLEyeLandsUsed.resize(nShapes);
fExtraREyeLandsUsed.resize(nShapes);
for (int iShape = 0; iShape < nShapes; iShape++)
    {
    fExtraLEyeLandsUsed[iShape] = false;
    if (OrgShapes[iShape].nrows() > MLEye0 &&
            fPointUsed(OrgShapes[iShape], MLEye0))
        {
        fExtraLEyeLandsUsed[iShape] = true;
        }
    fExtraREyeLandsUsed[iShape] = false;
    if (OrgShapes[iShape].nrows() > MREye0 &&
            fPointUsed(OrgShapes[iShape], MREye0))
        {
        fExtraREyeLandsUsed[iShape] = true;
        }
    }
}

//-----------------------------------------------------------------------------
static bool fMustSynthProf (
    const int iShape,                   // in
    const int iPoint,                   // in
    const vec_bool fExtraLEyeLandsUsed,  // in
    const vec_bool fExtraREyeLandsUsed,  // in
    const vec_int  TagInts)              // in
{
bool fSynth = false;

// Can we synthesize left eye points from right eye?
// Only if left eye point is unused and right eye point is avaliable.

if (iPoint >= MLEye0 && iPoint <= MLEye7 &&
        !fExtraLEyeLandsUsed[iShape] &&
        fExtraREyeLandsUsed[iShape])
    fSynth = true;

// Can we synthesize right eye points from left eye?
// Only if right eye point is unused and left eye point is avaliable.

if (iPoint >= MREye0 && iPoint <= MREye7 &&
        !fExtraREyeLandsUsed[iShape] &&
        fExtraLEyeLandsUsed[iShape])
    fSynth = true;

if (fPointObscured(TagInts, iShape, iPoint))
    fSynth = false;

return fSynth;
}

//-----------------------------------------------------------------------------
static void SynthProf (Vec &NewProf,        // out
                       const Vec &OldProf,  // in
                       unsigned ProfSpec)   // in
{
CheckSameDim(NewProf, OldProf, "SynthProf");

if (IS_2D(ProfSpec))                        // two dimensional prof?
    {
    // copy OldProf into NewProf but reverse the order of each row

    MatView OldSquare(OldProf.viewAsSquare());
    MatView NewSquare(NewProf.viewAsSquare());
    const int nRows = OldSquare.nrows();
    const int nCols = OldSquare.ncols();
    for (int iRow = 0; iRow < nRows; iRow++)
        for (int iCol = 0; iCol < nCols; iCol++)
            NewSquare(iRow, iCol) = OldSquare(iRow, nCols - iCol - 1);
    }
else
    {
    // Copy OldProf into NewProf (no reversal is needed because
    // whisker directions chosen so that we don't need to mirror 1D profiles).
    // Whisker directions are determined by iPrev and iNext in gLandTab

    NewProf = OldProf;
    }
}

//-----------------------------------------------------------------------------
// This is the brother function to CollectProfForOneLandmark but instead
// of collecting the profile for iPoint from the image, we get synthesize
// the profile i.e. get the profile from the partner point's profile.

void SynthProfFromPartner (Mat &Profs,              // io
                           Mat &PartnerProfs,       // io
                           int &nProfs,             // io
                           int &nPartnerProfs,      // io
                           int iLev,                // in
                           int iPoint)              // in
{
ASSERT(CONF_fXm2vts);   // this function uses gLandTab
const int iPartner = gLandTab[iPoint].iPartner;
ASSERT(iPartner > 0);

const unsigned Bits = gLandTab[iPoint].Bits;
ASSERT(Bits & FA_Synthesize);
const unsigned PartnerBits = gLandTab[iPartner].Bits;
ASSERT(Bits == PartnerBits);

// Using symmetry to generate landmarks only works if whiskers are
// symmetrical, so fgExplicitPrevNext must be true.
// This only applies to 1D profiles because 2D profiles don't use
// whiskers.  So check here that this condition if true.

if ((CONF_fExplicitPrevNext == false) && !IS_2D(GetGenProfSpec(iLev, iPoint)))
    Err("SynthProfFromPartner: 1D synthesized profile yet "
        "CONF_fExplicitPrevNext==false, iLev %d iPoint %d", iLev, iPoint);

int iProf = nProfs;                 // start where we left off last time

unsigned ProfSpec = GetGenProfSpec(iLev, iPoint);
ASSERT(ProfSpec == GetGenProfSpec(iLev, iPartner));

Mat *pProfs = &Profs;               // profiles of current point
Mat *pPartnerProfs = &PartnerProfs; // profiles of corresponding partner
#if _MSC_VER // microsoft
    SynthProf(pProfs->row(iProf), pPartnerProfs->row(iProf), ProfSpec);
#else        // TODO Temporary var needed for gcc. Its slow.
    Mat Prof(pProfs->row(iProf));
    SynthProf(Prof, pPartnerProfs->row(iProf), ProfSpec);
    pProfs->row(iProf) = Prof;
#endif

nProfs = ++iProf;

if (nProfs != nPartnerProfs)
    Err("SynthProfFromPartner: iLev %d iPoint %d nProfs %d != nPartnerProfs %d",
        iLev, iPoint, nProfs, nPartnerProfs);
}

//-----------------------------------------------------------------------------
void SynthEyeProfs (vec_Mat &Profs,                     // io
                    vec_int &nProfs,                    // io
                    const int nPoints,                  // in
                    const int iShape,                   // in
                    const int iLev,                     // in
                    const vec_bool fExtraLEyeLandsUsed, // in
                    const vec_bool fExtraREyeLandsUsed, // in
                    const vec_int TagInts)              // in
{
for (int iPoint = 0; iPoint <  nPoints; iPoint++)
    if (fMustSynthProf(iShape, iPoint,
            fExtraLEyeLandsUsed, fExtraREyeLandsUsed, TagInts))
        {
        const int iPartner = gLandTab[iPoint].iPartner;

        SynthProfFromPartner(Profs[iPoint], Profs[iPartner],
                nProfs[iPoint], nProfs[iPartner],
                iLev, iPoint);
        }
}
