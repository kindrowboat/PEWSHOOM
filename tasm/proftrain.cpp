// $proftrain.cpp 3.0 milbo$ training routines for ASM profiles
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
// Return the 2D profile type that must be generated for this level.
// Returns 0 if there are no 2D profiles at this level.
//
// This calls GetGenProfSpec() to see what type of gradient to generate.
// GetGenProfSpec is part of the model configuration.
//
// For now, all the 2D profile types must be the same because we store
// Grads on an image basis, not on a landmark basis (but we do create distinct
// Grads for each level in the image)
// So here we check that they do indeed all match and return their common value.

static unsigned GetGeneratedProfType2dForThisLev (int iLev, int nPoints)
{
unsigned ProfSpec = 0;
for (int iPoint = 0; iPoint < nPoints; iPoint++)
    {
    unsigned ProfSpec1 = GetGenProfSpec(iLev, iPoint);
    if (IS_2D(ProfSpec1))
        {
        if (ProfSpec == 0)
            ProfSpec = ProfSpec1;
        else if (ProfSpec1 != ProfSpec)
            Err("ProfSpec not constant over all 2D profiles");
        }
    }
return ProfSpec;
}

//-----------------------------------------------------------------------------
void InitGradsIfNeededForModelGeneration (Mat &Grads,   // out:
            const Image &Img,       // in: Img already scaled to this pyr lev
            int iLev,               // in
            int nPoints)            // in
{
ASSERT(iLev < 16);  // following init relies on iLev less than 16

// Cache the 2d prof specs so we don't have to recalculate every time.
// This code relies on the fact that 2d profiles for all landmarks in
// a given level are the same.
// See also CheckThatAll2dProfsMatch().

static unsigned ProfSpecs2d[16] =
                    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

if (ProfSpecs2d[iLev] == unsigned(-1))    // first time? if so must initialize
    ProfSpecs2d[iLev] = GetGeneratedProfType2dForThisLev(iLev, nPoints);

if (IS_2D(ProfSpecs2d[iLev]))
    InitGrads(Grads, Img, ProfSpecs2d[iLev]);
}

//-----------------------------------------------------------------------------
// This specifies the profile type to use when generating the model.
//
// This function is used during training and not during search
// So the following apply when GENERATING profile data (that's why
// there is a "Gen" in the function name).
// When USING profile data from the model file (i.e during search) we
// use the profile type specified in the .asm file.

unsigned GetGenProfSpec (int iLev, int iPoint)
{
bool fTwoD = false;
if (iLev <= CONF_nLev2d)
    {
    switch (CONF_nWhich2d)
        {
        case GEN2D_All:
            fTwoD = true;
            break;
        case GEN2D_InternalExceptMouth:
            fTwoD = (iPoint >= MROuterEyeBrow && iPoint < MLMouthCorner) ||
                    iPoint > MMouthBotOfTopLip;
            break;
        case GEN2D_Internal:
            fTwoD = iPoint >= MROuterEyeBrow;
            break;
        case GEN2D_Eyes:
            fTwoD = (iPoint >= MLEyeOuter && iPoint <= MREye);
            break;
        case GEN2D_EyesExt: // XM2VTS eyes plus extended XM2VTS points
            fTwoD = (iPoint >= MLEyeOuter && iPoint <= MREye) ||
                    (iPoint >= MLEye0 && iPoint <= MREye7);
            break;
        case GEN2D_InternalExceptNose:
            fTwoD = iPoint >= MROuterEyeBrow &&
                    (iPoint < 38 || iPoint > 44) &&
                    iPoint != 67;
            break;
        default:
            Err("bad nWhich2d %d", CONF_nWhich2d);
        }
    }
if (fTwoD)
    {
    ASSERT((CONF_ProfType2d & ~0xff0000ff) == 0);
    return PROF_2d | CONF_ProfType2d;
    }
else
    return CONF_ProfType;   // one dimensional profile
}

//-----------------------------------------------------------------------------
// This is only needed when GENERATING a model.
// When USING the model (i.e. during search), use nGetProfWidth() instead.

int nGenElemsPerSubProf (unsigned ProfSpec)
{
if (!IS_2D(ProfSpec))       // one dimensional profile?
    return CONF_nProfWidth1d;

return CONF_nProfWidth2d * CONF_nProfWidth2d;
}
