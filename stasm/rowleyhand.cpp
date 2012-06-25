// $rowleyhand.cpp 3.0 milbo$ hand tuned startshape from Rowley detector
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

// Parameters were tuned on a validation set (independent of the
// training set and the test set).  See the master's thesis.

static const double CONF_RowleyHeightRatio  = 0.60;
static const double CONF_RowleyWidthRatio   = 0.07;
static const double CONF_RowleyEyeWeight    = 1.0;
static const double CONF_RowleyWidthWeight  = 0.5;
static const double CONF_RowleyHeightWeight = 0.5;
static const double CONF_MaxEyeToFaceWidth  = 0.52;
static const double CONF_EyeFaceBlend       = 0.60;

//-----------------------------------------------------------------------------
// create (pre stasm version 1.7) DetShape from DetParams

SHAPE DetParamsToShapeOld (const DET_PARAMS &DetParams)
{
SHAPE DetShape(4, 2);

double w = DetParams.width / 2;             // half width of face box
double h = DetParams.height / 2;            // half height of face box

DetShape(DET_TopLeft, VX) = DetParams.x - w;
DetShape(DET_TopLeft, VY) = DetParams.y + h;
DetShape(DET_BotRight, VX) = DetParams.x + w;
DetShape(DET_BotRight, VY) = DetParams.y - h;
DetShape(DET_LEye, VX) = DetParams.lex;
DetShape(DET_LEye, VY) = DetParams.ley;
DetShape(DET_REye, VX) = DetParams.rex;
DetShape(DET_REye, VY) = DetParams.rey;

return DetShape;
}

//-----------------------------------------------------------------------------
// align MeanShape to the Rowley detector eyes (ignore the overall face box)

static void
AlignToRowleyEyes (SHAPE &StartShape,                              // out
                   const SHAPE &MeanShape, const SHAPE &DetShape)  // in
{
SHAPE Base(2, 2), Rowley(2, 2);   // points are MLEye MrEye

ASSERT(MLEye < int(MeanShape.nrows()) && MREye < int(MeanShape.nrows()));

Base.row(0) = MeanShape.row(MLEye);
Base.row(1) = MeanShape.row(MREye);

// in practice the Rowley detector sometimes misaligns the eyes so here
// it's better to just use the average height of the eyes

double yMean = (DetShape(DET_LEye, VY) + DetShape(DET_REye, VY)) / 2;
Rowley(0, VX) = DetShape(DET_LEye, VX);
Rowley(0, VY) = yMean;
Rowley(1, VX) = DetShape(DET_REye, VX);
Rowley(1, VY) = yMean;

ASSERT(!Base.fARowIsZero());
ASSERT(!Rowley.fARowIsZero());

Mat Pose(AlignShape(Base, Rowley));

StartShape = TransformShape(MeanShape, Pose);
}

//-----------------------------------------------------------------------------
// align MeanShape to the Rowley detector overall face box and the eyes

static void
AlignToRowleyFaceAndEyes (SHAPE &StartShape,            // out
        const SHAPE &MeanShape, const SHAPE &DetShape)  // in
{
SHAPE Base(4, 2), Rowley(4, 2);   // points are MLEye MrEye MlJaw2 MRJaw2

ASSERT(MLEye < int(MeanShape.nrows()) && MREye < int(MeanShape.nrows()));

Base.row(0) = MeanShape.row(MLEye);
Base.row(1) = MeanShape.row(MREye);
Base.row(2) = MeanShape.row(MLJaw2);
Base.row(3) = MeanShape.row(MRJaw2);

ASSERT(!Base.fARowIsZero());

// in practice the Rowley detector sometimes misaligns the eyes so here
// it's better to just use the average height of the eyes

double yMean = (DetShape(DET_LEye, VY) + DetShape(DET_REye, VY)) / 2;
Rowley(0, VX) = DetShape(DET_LEye, VX);
Rowley(0, VY) = yMean;
Rowley(1, VX) = DetShape(DET_REye, VX);
Rowley(1, VY) = yMean;

double xMin = DetShape(DET_TopLeft, VX);
double xMax = DetShape(DET_BotRight, VX);

// recenter the face on the midpoint of eyes (found to give better results)

double EyeMidpoint = (DetShape(DET_LEye, VX)+DetShape(DET_REye, VX))/2;
xMin = EyeMidpoint - (xMax - xMin)/2;
xMax = EyeMidpoint + (xMax - xMin)/2;

double yMin = DetShape(DET_TopLeft, VY);
double yMax = DetShape(DET_BotRight, VY);

// y position of Jaw2 in Rowley box
double HeightCorrect = CONF_RowleyHeightRatio * (yMax - yMin);

// difference between MLJaw2-to-MRJaw2 and Rowley box
double WidthCorrect  = CONF_RowleyWidthRatio * (xMax - xMin);

Rowley(2, VX) = xMin + WidthCorrect;   Rowley(2, VY) = yMin + HeightCorrect;
Rowley(3, VX) = xMax - WidthCorrect;   Rowley(3, VY) = yMin + HeightCorrect;

ASSERT(!Rowley.fARowIsZero());

Vec Weights(Base.nrows());
Weights(0) = CONF_RowleyEyeWeight;
Weights(1) = CONF_RowleyEyeWeight;
Weights(2) = CONF_RowleyWidthWeight;
Weights(3) = CONF_RowleyHeightWeight;

Mat Pose(AlignShape(Base, Rowley, &Weights));

StartShape = TransformShape(MeanShape, Pose);
}

//-----------------------------------------------------------------------------
// Align StartShape to DetParams, which are parameters returned by the
// Rowley detector.  The "Hand" in the name refers to the fact that
// this routine was hand tweaked to give good results.
// This gives the same results as Stasm version 1.5.

void
AlignToHandRowley (SHAPE &StartShape,               // out
                   const DET_PARAMS &DetParams,     // in
                   const SHAPE &MeanShape)          // in
{
ASSERT(CONF_fXm2vts);
ASSERT(MeanShape.nelems() > 0 || fPointUsed(MeanShape, 0));

SHAPE DetShape = DetParamsToShapeOld(DetParams);
AlignToRowleyFaceAndEyes(StartShape, MeanShape, DetShape);

// Align the MeanShape using only the Rowley eye positions, to give
// the shape StartEyes.

SHAPE StartEyes;
AlignToRowleyEyes(StartEyes, MeanShape, DetShape);

// Measure the eye positions in StartEyes against those in the StartShape
// we got above (which was aligned to both the detector face and detector eyes).
// If the eye positions are very different, then form a new StartShape by blending
// StartShape and StartEyes.
//
// This has proven effective in improving the fit in faces with large alignment
// errors, without affecting other faces.

double EyeToFaceWidth =
    (StartEyes(MREye, VX) - StartEyes(MLEye, VX)) /
    (StartShape(MRJaw2, VX) - StartShape(MLJaw2, VX));

double Blend = 0;   // assume no blending
if (EyeToFaceWidth > CONF_MaxEyeToFaceWidth)
    Blend = CONF_EyeFaceBlend;

int i;
for (i = 0; i <= MNoseTip; i++)
    StartShape.row(i) = (1-Blend) * StartShape.row(i) + Blend * StartEyes.row(i);

const int nPoints = StartShape.nrows();
for (i = MLEye0; i < nPoints; i++)   // extra eye points
    StartShape.row(i) = (1-Blend) * StartShape.row(i) + Blend * StartEyes.row(i);
}
