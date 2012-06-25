// $imshape.cpp 3.0 milbo$ routines for using shapes in images
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

// if true, skip lines connecting extra eye points >= 68 in DrawShape
// i.e. if true, draw the shape using the same points as an XM2VTS shape

static const bool SKIP_EXTRA_EYE_LINES_IN_DISPLAYED_SHAPE = true;

// shapes wider than this will be drawn with thicker lines, makes line more visible

int igThickenShapeWidth = 300;

//-----------------------------------------------------------------------------
double GetPyrScale (int iLev, double PyrRatio)
{
return pow(PyrRatio, iLev);
}

//-----------------------------------------------------------------------------
void
DrawPoint (RgbImage &Img,                                   // io
           int ix, int iy,                                  // in
           unsigned Red, unsigned Green, unsigned Blue,     // in
           bool fTransparent)                               // in
{
int width = Img.width, height = Img.height;
ix += width / 2;       // convert shape to image coords
iy += height / 2;
int i =  ix + iy * width;
if (i > 0 && i < width * height)
    {
    if (fTransparent)
        {
        Img(ix, iy).Red   = byte(Img(ix, iy).Red / 2   + Red / 2);
        Img(ix, iy).Green = byte(Img(ix, iy).Green / 2 + Green / 2);
        Img(ix, iy).Blue  = byte(Img(ix, iy).Blue / 2  + Blue / 2);
        }
    else
        {
        Img(ix, iy).Red = byte(Red);
        Img(ix, iy).Green = byte(Green);
        Img(ix, iy).Blue = byte(Blue);
        }
    }
}

//-----------------------------------------------------------------------------
// same as above but with double arguments

void
DrawPoint (RgbImage &Img,                                   // io
           double x, double y,                              // in
           unsigned Red, unsigned Green, unsigned Blue,     // in
           bool fTransparent)                               // in
{
DrawPoint(Img, iround(x), iround(y), Red, Green, Blue, fTransparent);
}

//-----------------------------------------------------------------------------
void
DrawCross (RgbImage &Img,                               // io
           double x, double y,                          // in
           unsigned Red, unsigned Green, unsigned Blue) // in
{
DrawPoint(Img, x-2, y-2, Red, Green, Blue);
DrawPoint(Img, x-1, y-1, Red, Green, Blue);
DrawPoint(Img, x-1, y+1, Red, Green, Blue);
DrawPoint(Img, x-2, y+2, Red, Green, Blue);
DrawPoint(Img, x+2, y-2, Red, Green, Blue);
DrawPoint(Img, x+1, y-1, Red, Green, Blue);
DrawPoint(Img, x+1, y+1, Red, Green, Blue);
DrawPoint(Img, x+2, y+2, Red, Green, Blue);
}

//-----------------------------------------------------------------------------
// This function uses a Bresenham-like algorithm to draw a line from x0,y0
// to x1,y1.  Based on "Bresenham-based supercover line algorithm" by
// Eugen Dedu www.ese-metz.fr/~dedu/projects/bresenham.

static void
DrawLine (RgbImage &Img,                            // io
    int x0, int y0, int x1, int y1,                 // in
    unsigned Red, unsigned Green, unsigned Blue,    // in
    bool fTransparent=false,                        // in
    bool fThick=false)                              // in
{
int i;               // loop counter
int yStep, xStep;    // the step on y and x axis
int Error;           // the error accumulated during the increment
int ddy, ddx;
int x = x0, y = y0;
int dx = x1 - x0;
int dy = y1 - y0;

DrawPoint(Img, x, y, Red, Green, Blue, fTransparent); // first point
if (fThick)
    DrawPoint(Img, x+1, y+1, Red, Green, Blue, fTransparent);

if (dy < 0)
    {
    yStep = -1;
    dy = -dy;
    }
else
    yStep = 1;

if (dx < 0)
    {
    xStep = -1;
    dx = -dx;
    }
else
    xStep = 1;

ddy = 2 * dy;
ddx = 2 * dx;

if (ddx >= ddy)                 // first octant (0 <= slope <= 1)
    {
    Error = dx;                 // start in the middle of the square
    for (i = 0; i < dx; i++)    // do not use the first point (already done)
        {
        x += xStep;
        Error += ddy;
        if (Error > ddx)        // increment y if AFTER the middle (>)
            {
            y += yStep;
            Error -= ddx;
            }
        DrawPoint(Img, x, y, Red, Green, Blue, fTransparent);
        if (fThick)
            DrawPoint(Img, x+yStep, y+xStep, Red, Green, Blue, fTransparent);
        }
    }
else
    {
    Error = dy;
    for (i = 0; i < dy; i++)
        {
        y += yStep;
        Error += ddx;
        if (Error > ddy)
            {
            x += xStep;
            Error -= ddy;
            }
        DrawPoint(Img, x, y, Red, Green, Blue, fTransparent);
        if (fThick)
            DrawPoint(Img, x+yStep, y+xStep, Red, Green, Blue, fTransparent);
        }
    }
}

//-----------------------------------------------------------------------------
static void GetPrevAndNext (int &iPrev, int &iNext,              // out
                            int iPoint, const SHAPE &Shape)      // in
{
const int nPoints = Shape.nrows();
iNext = iPoint;
int nCount = 0; // safety counter, needed if no points are used
do
    {
    iNext++;
    if (iNext >= nPoints)
        iNext = 0;
    }
while(nCount++ < nPoints && !fPointUsed(Shape, iNext));

iPrev = iPoint;
nCount = 0;
do
    {
    iPrev--;
    if (iPrev < 0)
        iPrev = nPoints-1;
    }
while(nCount++ < nPoints && !fPointUsed(Shape, iPrev));
}

//-----------------------------------------------------------------------------
// Get the coords of iPoint
// If iPoint is not used, synthesize the coords from the previous and next points.

void GetPointCoords (double &x, double &y,                        // out
                     int iPoint, const SHAPE &Shape)             // in
{
if (fPointUsed(Shape, iPoint))
    {
    x = Shape(iPoint, VX);
    y = Shape(iPoint, VY);
    }
else
    {
    int iPrev, iNext;
    GetPrevAndNext(iPrev, iNext, iPoint, Shape);
    int iPrev1 = iPrev, iNext1 = iNext;   // handle wraparound
    if (iPrev > iPoint)
        iPrev1 -= Shape.nrows();
    if (iNext < iPoint)
        iNext1 += Shape.nrows();
    double ScalePrev = double(iNext1 - iPoint) / (iNext1 - iPrev1);
    double ScaleNext = double(iPoint - iPrev1) / (iNext1 - iPrev1);
    x = ScalePrev * Shape(iPrev, VX) + ScaleNext * Shape(iNext, VX);
    y = ScalePrev * Shape(iPrev, VY) + ScaleNext * Shape(iNext, VY);
    }
}

//-----------------------------------------------------------------------------
// Reorder the points of a 76 point shape to display the eye landmarks nicely.
// We assume here that a 76 point shape must be a shape from the MUCT database.

SHAPE End76ToMid76 (const SHAPE &Shape)
{
ASSERT(Shape.nrows() == 76);
static SHAPE Shape1(76, 2);
for (unsigned i = 0; i < Shape1.nrows(); i++)
    {
    int j = iEnd76ToMid76[i];
    Shape1(i, VX) = Shape(j, VX); Shape1(i, VY) = Shape(j, VY);
    }
return Shape1;
}

//-----------------------------------------------------------------------------
// Draw Shape in the image Img in the specified Color
// If a point's position is 0,0 then assume that it is marked as invisible
// and plot the point midway between the points on either side of it.

void DrawShape (RgbImage &Img,               // io
        const SHAPE &Shape,                  // in
        unsigned Color,                      // in: color of landmarks
        bool fConnectTheDots,                // in
        unsigned ConnectTheDotsColor,        // in: -1 means use default dark red
        bool fAnnotate,                      // in: add landmark numbers
        int iAnnotateLandmark)               // in: landmark to annotate, -1 for none
{
const bool fThick = (xShapeExtent(Shape) > igThickenShapeWidth);
if (ConnectTheDotsColor == unsigned(-1))      // use default color?
    ConnectTheDotsColor = C_DRED;             // dark red
SHAPE Shape1(Shape);
if (Shape.nrows() == 76)                    // hack to display 76 point shapes nicely
    Shape1 = End76ToMid76(Shape1);
const int nPoints = Shape1.nrows();
double x, y;

// connect the dots

if (fConnectTheDots)
    {
    const unsigned RedConnect   = (ConnectTheDotsColor >> 16) & 0xff;
    const unsigned GreenConnect = (ConnectTheDotsColor >> 8) & 0xff;
    const unsigned BlueConnect  = ConnectTheDotsColor & 0xff;
    for (int iPoint = 0; iPoint < nPoints; iPoint++)
        {
        int iPrev, iNext;
        GetPointCoords(x, y, iPoint, Shape1);
        GetPrevAndNext(iPrev, iNext, iPoint, Shape1);
        if (iNext > iPoint) // don't join last point to first
            {
            DrawLine(Img, iround(x), iround(y),
                iround(Shape1(iNext, VX)), iround(Shape1(iNext, VY)),
                RedConnect, GreenConnect, BlueConnect,
                true, fThick);
            }
        }
    }

// annotate

if (fAnnotate || iAnnotateLandmark != -1)
    {
#if _MSC_VER // currently only the microsoft builds supports RgbPrintf
    BeginRgbPrintf(Img);
    for (int iPoint = 0; iPoint < nPoints; iPoint++)
        {
        int iPoint1 = iPoint;
        if (fAnnotate || iPoint == iAnnotateLandmark)
            {
            GetPointCoords(x, y, iPoint1, Shape);
            unsigned TextColor;
            int iFontSize = fThick? 150: 70;
            static const int EZ_ATTR_BOLD = 1;  // TODO lifted from ezfont.h
            int iAttributes = fThick? EZ_ATTR_BOLD: 0;
            if (iPoint1 == iAnnotateLandmark)
                {
                TextColor = C_YELLOW;
                iFontSize = fThick? 330: 100;
                }
            else
                TextColor = C_RED;

//             // alternate the colors: gaudy but easier to
//             // make out the numbers when crowded
//
//             if (nPoints > 25 && iAnnotateLandmark < 0 && iPoint1 % 2 == 0)
//                 TextColor = C_YELLOW;

            DoRgbPrintf(int(Img.width/2 + x + 2), int(Img.height/2 - y),
                        TextColor, iFontSize, "%d", iPoint1);
            }
        }
    EndRgbPrintf(Img);
#else
    WarnWithNewLine("DrawShape(fAnnotate=true) can only be used under Windows");
#endif
    }

// draw the points last so they are over over the lines and annotations

const unsigned Red   = (Color >> 16) & 0xff;
const unsigned Green = (Color >> 8) & 0xff;
const unsigned Blue  = Color & 0xff;

for (int iPoint = 0; iPoint < nPoints; iPoint++)
    {
    GetPointCoords(x, y, iPoint, Shape1);
    DrawPoint(Img, x, y, Red, Green, Blue);
    if (fThick)
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++)
                DrawPoint(Img, x+i, y+j, Red, Green, Blue);
    if (!fPointUsed(Shape1, iPoint))
        DrawCross(Img, x, y, Red, Green, Blue);
    }
}

//-----------------------------------------------------------------------------
// crop the image so the shape fills the whole image

void
CropImageToShape (RgbImage &Img,                  // io
                  const SHAPE &Shape,             // in
                  int ixMargin,                   // in
                  int iyMargin)                   // in
{
int Width = (int)xShapeExtent(Shape);

// increase the default margins below if you want more room around the shape
// TODO would be nice to leave more room around the top of the face, to get hair

double xMargin = (ixMargin == 0)? Width / 6: ixMargin;
double yMargin = (iyMargin == 0)? Width / 5: iyMargin;

int nLeftCrop   = iround(MAX(0, Img.width/2  + Shape.col(VX).minElem() - xMargin));
int nRightCrop  = iround(MAX(0, Img.width/2  - Shape.col(VX).maxElem() - xMargin));
int nTopCrop    = iround(MAX(0, Img.height/2 - Shape.col(VY).maxElem() - yMargin));
int nBottomCrop = iround(MAX(0, Img.height/2 + Shape.col(VY).minElem() - yMargin));

CropRgbImage(Img,
             nTopCrop, nBottomCrop, nLeftCrop, nRightCrop,
             IM_WIDTH_DIVISIBLE_BY_4);
}

//-----------------------------------------------------------------------------
// Convert Shape to Shape17 which has just the  me17 points

void ConvertToShape17 (SHAPE &Shape17,         // out
                       const SHAPE &Shape)     // in
{
Shape17.dim(17, 2);
const int nPoints = Shape.nrows();

if (nPoints >= 68 && nPoints < 100)        // assume XM2VTS
    {
    for (int iPoint = 0; iPoint < nPoints; iPoint++)
        {
        const int i = igMe17s[iPoint];
        if (i >= 0)
            {
            Shape17(i, VX) = Shape(iPoint, VX);
            Shape17(i, VY) = Shape(iPoint, VY);
            }
        }
    }
else if (nPoints == 20 || Shape.nrows() == 22)    // assume BioId or AR
    {
    int i = 0;
    for (int iPoint = 0; iPoint < nPoints; iPoint++)
        switch(iPoint)
            {
            case 8:
            case 13:
            case 19:
            case 20:
            case 21:
                break;
            default:
                Shape17(i, VX) = Shape(iPoint, VX);
                Shape17(i, VY) = Shape(iPoint, VY);
                i++;
                break;
            }
    ASSERT(i == 17);
    }
else if (nPoints == 199)    // assume Put199
    {
    for (int iPoint = 0; iPoint < 17; iPoint++)
        {
        Shape17(iPoint, VX) = Shape(iPut199Me17s[iPoint], VX);
        Shape17(iPoint, VY) = Shape(iPut199Me17s[iPoint], VY);
        }
    }
else
    Err("don't know how to convert shape with %d rows to me17", nPoints);
}
