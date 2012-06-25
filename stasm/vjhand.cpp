// $vjhand.cpp 3.0 milbo$ hand tuned startshape from ViolaJones detector
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
// Align StartShape to DetParams, which are parameters returned by the
// ViolaJones detector.  This requires the eye positions.
//
// The central idea is to form a triangular shape of the eyes and tip-of-chin
// from both the DetAv and the face detector params, then align the shapes.

void
AlignToHandViolaJones (SHAPE &StartShape,           // out
                       const DET_PARAMS &DetParams, // in
                       const SHAPE &DetAv)          // in
{
ASSERT(CONF_fXm2vts);                   // this routine needs the eye positions
ASSERT(DetAv.nelems() > 0 && fPointUsed(DetAv, 0));
ASSERT(DetParams.lex != INVALID && DetParams.rex != INVALID);

SHAPE MeanTri(3, 2), DetTri(3, 2);      // triangle of eyes and tip of chin

MeanTri(0, VX) = DetAv(MLEye, VX);      MeanTri(0, VY) = DetAv(MLEye, VY);
MeanTri(1, VX) = DetAv(MREye, VX);      MeanTri(1, VY) = DetAv(MREye, VY);
MeanTri(2, VX) = DetAv(MTipOfChin, VX); MeanTri(2, VY) = DetAv(MTipOfChin, VY);

const double yTip = DetParams.y - DetParams.width / 2;
DetTri(0, VX) = DetParams.lex; DetTri(0, VY) = DetParams.ley;   // left eye
DetTri(1, VX) = DetParams.rex; DetTri(1, VY) = DetParams.rey;   // right eye
DetTri(2, VX) = DetParams.x;   DetTri(2, VY) = yTip;            // tip of chin

Mat Pose(AlignShape(MeanTri, DetTri));

StartShape = TransformShape(DetAv, Pose);
}
