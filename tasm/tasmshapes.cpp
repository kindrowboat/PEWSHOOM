// $tasmshapes.cpp 3.0 milbo$ shape model functions for tasm
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
static int nGetNbrUsedPoints (const SHAPE &Shape, int nPoints)
{
int nUsed = 0;

for (int iPoint = 0; iPoint < nPoints; iPoint++)
    if (fPointUsed(Shape, iPoint))
        nUsed++;

return nUsed;
}

//-----------------------------------------------------------------------------
// This allocates *pfUnused, and the caller must free it

static void SaveUnusedPoints (bool **ppfUnused,                 // out
                              const SHAPE &Shape, int nRows)    // in
{
bool *pfUnused = (bool *)malloc(nRows * sizeof(bool *));
*ppfUnused = pfUnused;
for (int iRow = 0; iRow < nRows; iRow++)
    *pfUnused++ = !fPointUsed(Shape, iRow);
}

//-----------------------------------------------------------------------------
static void RestoreUnusedPoints (SHAPE &Shape,                          // out
                                    const bool afUnused[], int nRows)   // in
{
for (int iRow = 0; iRow < nRows; iRow++)
    if (afUnused[iRow])
        {
        Shape(iRow, VX) = 0;
        Shape(iRow, VY) = 0;
        }
}

//-----------------------------------------------------------------------------
// Centralize shape so its centroid is in the middle of the image.
// This knows about unused points, and makes sure they remain at 0,0
// after centralization.

static void CentralizeShape (SHAPE &Shape)  // io
{
bool *afUnused;
int nRows = Shape.nrows();
SaveUnusedPoints(&afUnused, Shape, nRows);  // remember which points are 0,0

int nUsed = nGetNbrUsedPoints(Shape, nRows);
double xAv = Shape.col(VX).sum() / nUsed;
double yAv = Shape.col(VY).sum() / nUsed;
Shape.col(VX) -= xAv;
Shape.col(VY) -= yAv;

RestoreUnusedPoints(Shape, afUnused, nRows); // restore original 0,0 points back to 0,0
free(afUnused);
}

//-----------------------------------------------------------------------------
// Add Shape to NewShape on a point by point basis.
// Same as NewShape += Shape, but with this difference: if a point isn't used
// in Shape, we use the corresponding point in the alternative
// shape AltShape instead

static void SumShapes (SHAPE &NewShape,                             // io
                       const SHAPE &Shape, const SHAPE &AltShape)   // in
{
CheckSameDim(NewShape, Shape, "SumShapes");
CheckSameDim(NewShape, AltShape, "SumShapes");

const int nPoints = Shape.nrows();
for (int iPoint = 0; iPoint < nPoints; iPoint++)
    {
    if (fPointUsed(Shape, iPoint))
        {
        NewShape(iPoint, VX) += Shape(iPoint, VX);
        NewShape(iPoint, VY) += Shape(iPoint, VY);
        }
    else
        {
        NewShape(iPoint, VX) += AltShape(iPoint, VX);
        NewShape(iPoint, VY) += AltShape(iPoint, VY);
        }
    }
}

//-----------------------------------------------------------------------------
// This aligns the shapes using the method in CootesTaylor 2004 section 4.2
// But we differ from that method as follows:
//
//   1. we don't normalize AvShape len to 1 (so can put aligned shapes on
//      top of images directly for debugging visualization)
//
//   2. we don't use tangent spaces
//
//   3. We ignore missing (i.e. 0,0) landmarks i.e. we can deal with
//      shapes with missing landmarks
//      The passed in RefShape must have all its landmarks, though.

static void AlignShapes (vec_SHAPE &Shapes,         // io
                         SHAPE &AvShape,            // io
                         const SHAPE &RefShape)     // in
{
static const int CONF_nMaxAlignPasses = 30;

int nShapes = Shapes.size();
int iShape;
for (iShape = 0; iShape < nShapes; iShape++)
    CentralizeShape(Shapes[iShape]);

// generate AvShape, the average shape

AvShape = RefShape;
int iPass;
for (iPass = 0; iPass < CONF_nMaxAlignPasses; iPass++)
    {
    SHAPE NewAvShape(AvShape.nrows(), AvShape.ncols());
    SHAPE AltShape(AvShape);    // alternate shape: see SumShapes
    for (iShape = 0; iShape < nShapes; iShape++)
        {
        AlignShape(Shapes[iShape], AvShape);
        SumShapes(NewAvShape, Shapes[iShape], AltShape);
        }
    // constrain new average shape by aligning it to the ref shape

    AlignShape(NewAvShape, RefShape);
    double Distance = AvShape.distSquared(NewAvShape);
    if (Distance < CONF_MaxShapeAlignDist)
        break;                  // converged
    AvShape = NewAvShape;
    if (Shapes.size() > 200)
        ReleaseProcessor();     // give others a chance
    }
if (iPass > CONF_nMaxAlignPasses)
    Err("could not align shapes in %d passes", CONF_nMaxAlignPasses);

// align all the shapes to the average shape

for (iShape = 0; iShape < nShapes; iShape++)
    AlignShape(Shapes[iShape], AvShape);
}

//-----------------------------------------------------------------------------
// Print the angle w.r.t. horizontal of the left and rightmost points in Shape.

static void ShowShapeAngle (const SHAPE &Shape) // in
{
// find the leftmost and rightmost points

double Left = FLT_MAX, Right = FLT_MIN;
int iLeft = -1, iRight = -1;
const int nPoints = Shape.nrows();
for (int iPoint = 0; iPoint < nPoints; iPoint++)
    {
    if (Shape(iPoint, VX) < Left)
        {
        Left = Shape(iPoint, VX);
        iLeft = iPoint;
        }
    else if (Shape(iPoint, VX) > Right)
        {
        Right = Shape(iPoint, VX);
        iRight = iPoint;
        }
    }
double Angle = atan2(Shape(iRight, VY) - Shape(iLeft, VY),
                     Shape(iRight, VX) - Shape(iLeft, VX));

lprintf("Outermost landmarks (%d,%d) angle %.3g degrees\n",
    iLeft, iRight, Degrees(Angle));
}

//-----------------------------------------------------------------------------
// Print the angle w.r.t. horizontal of the eyes in Shape.
// After aligning shapes, this angle should be near zero.

static void ShowEyeAngle (const SHAPE &Shape) // in
{
ASSERT(CONF_fXm2vts);

double Angle = atan2(Shape(MREye, VY) - Shape(MLEye, VY),
                     Shape(MREye, VX) - Shape(MLEye, VX));

lprintf("Eyes (%d,%d) angle %.3g degrees\n", MLEye, MREye, Degrees(Angle));
}

//-----------------------------------------------------------------------------
// align all Shapes, using the reference shape (at iRefShape) as a reference

static void AlignTrainingShapes (SHAPE            &AvShape,     // io
                                 vec_SHAPE        &Shapes,      // io
                                 const vec_string &Tags,        // in
                                 int              iRefShape)    // in
{
clock_t StartTime = clock();
const char *sShapeName = sGetBasenameFromTag(Tags[iRefShape]);
lprintf("Aligning shapes to %s",  sShapeName);

ASSERT(unsigned(iRefShape) < Shapes.size());

int nPoints = Shapes[iRefShape].nrows();
AvShape.dimKeep(nPoints, 2);

SHAPE RefShape(Shapes[iRefShape]);
CentralizeShape(RefShape);
AlignShapes(Shapes, AvShape, RefShape);

lprintf(" [%.1f secs]\n", double(clock() - StartTime) / CLOCKS_PER_SEC);

ShowShapeAngle(AvShape);
if (CONF_fXm2vts)
    ShowEyeAngle(AvShape);
}

//-----------------------------------------------------------------------------
static void AddNoiseToShape (SHAPE &Shape)  // io
{
if (CONF_xStretchShape != 0.0        ||
        CONF_xRhsStretchShape != 0.0 ||
        CONF_ShapeNoise != 0.0)
    {
    // add noise to the shape

    // rand val, 68% of values between +- CONF_xStretchShape
    double xRhsStretch = RandGauss(CONF_xRhsStretchShape);
    double xStretch = RandGauss(CONF_xStretchShape);

    const int nRows = Shape.nrows();
    for (int iRow = 0; iRow < nRows; iRow++)
        {
        if (Shape(iRow, VX) > 0)            // stretch rhs of shape horizontally
            Shape(iRow, VX) *= (1 + xRhsStretch);
        Shape(iRow, VX) *= (1 + xStretch);  // stretch entire shape horizontally
        Shape(iRow, VX) += RandGauss(CONF_ShapeNoise);  // add noise to x and y
        Shape(iRow, VY) += RandGauss(CONF_ShapeNoise);
        }
    }
}

//-----------------------------------------------------------------------------
// For improved consistency between _DEBUG and release versions, set very
// small eig values and their corresponding eig vecs to 0.  These small
// values are the unpredictable result of numerical errors.
// It's not clear why the GSL generates slightly different values
// for the _DEBUG and release versions with the Microsoft compiler.

static void ZapSmallEigs (Mat &EigVals, Mat &EigVecs)   // io
{
const int nRows = EigVals.nrows();
for (int iEig = 0; iEig < nRows; iEig++)
    if (EigVals(iEig) < EigVals(0) / 1E6)
        {
        EigVals(iEig) = 0;
        EigVecs.col(iEig).fill(0);
        }
}

//-----------------------------------------------------------------------------
// Print first n eig vals, up to and including the first small value.
// But no more than 10 of them.
// This is just for information.

static void ShowShapeEigs (const Mat &EigVals)
{
char s[SLEN];
unsigned iPrint = 0;

double MinEig = EigVals(0) / 1000;
while (iPrint < EigVals.nelems() && EigVals(iPrint) > MinEig && iPrint < 9)
    iPrint++;

if (iPrint < EigVals.nelems() + 1)  // show first small eig value, for context
    iPrint++;

Mat ScaledEigs = EigVals.t();
if (!fEqual(EigVals(0), 0))
    ScaledEigs /= EigVals(0);
else
    {
    sprintf(s, "%s%d scaled eigs: ",
        (iPrint < EigVals.nelems()? "First ": ""), iPrint);

    ScaledEigs.print(s, "%.2f ", NULL, NULL, iPrint);
    }
if (EigVals(0) < 0.1)               // number is somewhat arbitrary
    Warn("eigen data invalid "
         "(not enough variation in shapes, max eigenvalue is %g)", EigVals(0));
}

//-----------------------------------------------------------------------------
// This calculates the eigen vecs and values for Shapes.
// Shapes with a different number of landmarks from the reference
// shape are ignored.

static void CalcShapeEigs (Mat &EigVals, Mat &EigVecs,      // out
                           const vec_SHAPE &Shapes,         // in
                           const SHAPE &AvShape,            // in
                           const vec_int &nUsedLandmarks)   // in
{
int nPoints = AvShape.nrows();
int nShapes = Shapes.size();
int nShapesWithAllLandmarks = 0;
int iShape;

for (iShape = 0; iShape < nShapes; iShape++)
    if (nUsedLandmarks[iShape] == nUsedLandmarks[0])
        nShapesWithAllLandmarks++;

if (nShapesWithAllLandmarks != nShapes)
    lprintf("Used only %d of %d shapes for eigs because of missing landmarks\n",
        nShapesWithAllLandmarks, nShapes);

Mat ShapeDelta(nShapesWithAllLandmarks, 2 * nPoints);

// view all coords for AvShape as a single row vector, first x then y coords
VecView AvShapeAsRow(AvShape.viewAsRow());

SeedRand(CONF_nSeed_ShapeNoise);    // for AddNoiseToShape()

int iShape1 = 0;
for (iShape = 0; iShape < nShapes; iShape++)
    if (nUsedLandmarks[iShape] == nUsedLandmarks[0])
        {
        SHAPE Shape(Shapes[iShape]);
        AddNoiseToShape(Shape);
        ShapeDelta.row(iShape1) = Shape.viewAsRow() - AvShapeAsRow;
        iShape1++;
        }

Mat Covar((ShapeDelta.t() * ShapeDelta) / nShapesWithAllLandmarks);

EigVecs = GetEigsForSymMat(Covar, EigVals); // gets eig vecs sorted on eig vals

ZapSmallEigs(EigVals, EigVecs);

ShowShapeEigs(EigVals);
}

//-----------------------------------------------------------------------------
// Initialize eigvalues, eigvectors, and mean shape from the shapes

void GenShapeModel (Vec &EigVals,                   // out
                    Mat &EigVecs,                   // out
                    SHAPE &AvShape,                 // out
                    vec_SHAPE &Shapes,              // io: will be aligned
                    const int iRefShape,            // in
                    const vec_string &Tags,         // in
                    const vec_int &nUsedLandmarks)  // in
{
// align Shapes and calculate AvShape

AlignTrainingShapes(AvShape, Shapes,
                    Tags, iRefShape);

// build shape model i.e. calculate EigVals and EigVecs

CalcShapeEigs(EigVals, EigVecs,
              Shapes, AvShape, nUsedLandmarks);
}
