// $libthatch.cpp 3.0 milbo$ routines to generate a thatcher face
//
// The central routine in this file is Thatcherize().
//
// We thatcherize, say, a mouth, by vertically flipping the
// rectangle around the mouth.
//
// We get the position of the mouth from AsmSearch.  We then expand
// the bounding rectangle defined by the mouth landmarks (expand by
// WidthRatio and HeightRatio, see FlipRectangleWithBlend), so some space
// is included around the mouth, and flip that expanded rectangle.
//
// But simply flipping a rectangle leaves visible borders on the edges
// of the rectangle.  So instead we flip a series of nested
// rectangles of decreasing size.  The outermost flipped rectangle is
// blended with the original image, weighted towards the original
// image pixels.  As the rectangles decrease in size, we increase the
// blending ratio.  So the innermost rectangle is not blended (i.e.
// it is the flipped pixels with no blending in of the original
// image).  There are more efficient approaches --- but they
// are more complicated.
//
// milbo petaluma apr 2010

#include "stasm.hpp"
#include "../thatch/libthatch.hpp"

#define Blend(Color) Img(xTo, yTo).Color = \
                        byte(Blend * OriginalImg(xFrom, yFrom).Color + \
                        (1 - Blend) * OriginalImg(xTo, yTo).Color)

// The following structure defines the rectangle sizes and blending ratios.

typedef struct tBlends {
    int    iTrim;   // trim edge of bounding rectangle by this many pixels
    double Blend;   // 0 to 1
}  tBlends;

const tBlends gDefaultBlends[] = {  // default blend (quite a rough blend)
    { 0, .5 }, // outer rectangle pixels are a .5 blend of original and inverted rectangle
    { 2, 1  }, // inner rectangle is 2 pixels smaller and no blending
};

static const int ngCorner = 3; // trim corners of rectangles by this amount (0 for no trim)

static void RandomlyShiftFeatures(RgbImage &Img, const RgbImage &OriginalImg,
                          const SHAPE &Shape, bool fShowBox);

//-----------------------------------------------------------------------------
// Update Img by flipping the specified rectangular area in the image.
//
// Merge the flipped rectangle with the original image using the specified Blend:
//
//    NewPixel = Blend * PixelFromInvertedRectangle + (1-Blend) * PixelFromOriginalImage
//
// So use the default Blend=1 to overwrite the original rectangle with the inverted rectangle.
//
// TODO this is currently inefficient (would be better to access Img.buf directly).

static void FlipRectangle (RgbImage &Img, // io:
   const RgbImage &OriginalImg,     // in:
   int    ix, int iy,               // in: bottom left of bounding rectangle (in SHAPE coords)
   int    nWidth, int nHeight,      // in: size of bounding rectangle
   double Blend = 1,                // in:
   bool   fFlipHorizontal = false,  // in:
   bool   fFlipVertical = true,     // in:
   bool   fShowBox = false)         // in: show rectangle borders, for debugging
{
ix += Img.width  / 2;   // convert SHAPE to RgbImage coords
iy += Img.height / 2;

ASSERT(Img.width == OriginalImg.width);
ASSERT(Img.height == OriginalImg.height);
ASSERT(Blend >= 0 && Blend <= 1);

if (Blend > 0 && nWidth > 0 && nHeight > 0 &&
        ix >= 0 && ix < Img.width && iy >= 0 && iy < Img.height)
    for (int j = 0; j < nWidth; j++)
        for (int i = 0; i < nHeight; i++)
            {
            const int xTo = ix + j;
            const int yTo = iy + i;
            const int xFrom = (fFlipHorizontal? ix + nWidth  - j - 1: ix + j);
            const int yFrom = (fFlipVertical?   iy + nHeight - i - 1: iy + i);

            if (xTo < 0 || xTo >= Img.width || xFrom < 0 || xFrom >= Img.width ||
                    yTo < 0 || yTo >= Img.height || yFrom < 0 || yFrom >= Img.height)
                continue;

            if (ngCorner &&     // skip corners of rectangle?
                    (j < ngCorner         && i < ngCorner)          ||  // bottom left
                    (j < ngCorner         && i >= nHeight-ngCorner) ||  // top left
                    (j >= nWidth-ngCorner && i < ngCorner)          ||  // bottom right
                    (j >= nWidth-ngCorner && i >= nHeight-ngCorner))    // top right
                continue;

            if (Blend == 1)     // special case for efficiency
                Img(xTo, yTo) = OriginalImg(xFrom, yFrom);
            else
                {
                Blend(Red);
                Blend(Green);
                Blend(Blue);
                }
            if (fShowBox && (i == 0 || i == nHeight-1 || j == 0 || j == nWidth-1))
                {
                const RGB_TRIPLE Cyan = {255, 255, 0};
                Img(xTo, yTo) = Cyan;
                }
            }
}

//-----------------------------------------------------------------------------
// Flip a series of rectangles of decreasing size increasing blend.
// So in the outermost rectangle the flipped pixels are blended but
// retaining most of the original image, and the innermost rectangle is
// just the flipped pixels with none of the original image.
//
// The Blend parameter specifies the nbr of rectangles, their relative
// sizes, and their blending ratios.

static void FlipRectangleWithBlend (RgbImage &Img, // io:
   const RgbImage &OriginalImg,     // in:
   double x, double y,              // in: center of rectangle (in SHAPE coords)
   double Width, double Height,     // in: extent of rectangle
   double WidthRatio,               // in: expand Width by this amount
   double HeightRatio,              // in:
   const tBlends Blends[] = NULL,   // in: NULL means use gDefaultBlends
   int    nBlends = 0,              // in: nbr of elements in Blends
   double Expand=1,                 // in: adjust flipped area size
   bool   fFlipHorizontal= false,   // in: flip rectangle horizontally as well as vertically
   bool   fShowBox = false)         // in: show rectangle borders, for debugging
{
Width   *= Expand * WidthRatio;     // width  of bounding rectangle
Height  *= Expand * HeightRatio;    // height of bounding rectangle
int ix = int(x - .5 * Width);       // bottom of bounding rectangle
int iy = int(y - .5 * Height);      // left   of bounding rectangle
int iWidth = int(Width);
int iHeight = int(Height);

if (Blends == NULL)
    {
    Blends = gDefaultBlends;
    nBlends = NELEMS(gDefaultBlends);
    }
for (int iStep = 0; iStep < nBlends; iStep++)
    {
    int iTrim = Blends[iStep].iTrim;

    FlipRectangle(Img, OriginalImg,
                  ix + iTrim, iy + iTrim, iWidth - 2 * iTrim, iHeight - 2 * iTrim,
                  Blends[iStep].Blend, fFlipHorizontal, true,
                  fShowBox && iStep == 0);
    }
}

//-----------------------------------------------------------------------------
// Thatcherize the face in Img using the landmark positions in Shape
//
// NOTE: The constants in the routine below are just guesses really, based on
// testing on several images.

void Thatcherize (RgbImage &Img,    // io: the image with a face we are going to thatcherize
  const SHAPE &Shape,               // in: landmark results of ASM search
  double Expand,                    // in: adjust flipped area size
  bool   fMirrorMouth,              // in: mirror the mouth horizontally too
  bool   fRandomlyShiftFeatures,    // in: if true, randomly shift nose etc. slightly
  bool   fShowBox)                  // in: show bounding boxes of thatcherized areas for debugging
{
SHAPE Shape1(Shape);

ASSERT(Shape1.nrows() >= 68); // Ensure XM2VTS style shape because we use hard coded-indices into Shape1.
                             // For landmark numbering see e.g. Appendix A of Milborrow's masters thesis.
if (Shape1.nrows() > 68)
    Shape1.dimKeep(68, 2);

const RgbImage OriginalImg(Img);    // remains unchanged throughout

if (fRandomlyShiftFeatures)         // experimental, usually false
    RandomlyShiftFeatures(Img, OriginalImg, Shape1, fShowBox);

// left eye

const tBlends EyeBlends[] =
{
    { 0, .2 },
    { 3, .5 },
    { 5, .7 },
    { 6, 1  },
};

FlipRectangleWithBlend(Img,
   OriginalImg,
   (Shape1(29, VX) + Shape1(27, VX)) / 2,       // x coord of center of left eye
   (Shape1(28, VY) + Shape1(30, VY)) / 2,       // y coord of center of left eye
   Shape1(29, VX) - Shape1(27, VX),             // width of left eye
   Shape1(28, VY) - Shape1(30, VY),             // height of left eye
   1.5,                                         // WidthRatio (should match other eye below)
   2.6,                                         // HeightRatio (should match other eye below)
   EyeBlends, NELEMS(EyeBlends), Expand, false, fShowBox);

// right eye

FlipRectangleWithBlend(Img,
   OriginalImg,
   (Shape1(32, VX) + Shape1(34, VX)) / 2,
   (Shape1(33, VY) + Shape1(35, VY)) / 2,
   Shape1(32, VX) - Shape1(34, VX),
   Shape1(33, VY) - Shape1(35, VY),
   1.5,
   2.6,
   EyeBlends, NELEMS(EyeBlends), Expand, false, fShowBox);

// mouth

const tBlends MouthBlends[] =
{
    { 0, .2 },
    { 4, .4 },
    { 6, .6 },
    { 8, 1  },
};

const double MouthWidth = Shape1(54, VX) - Shape1(48, VX);
const double MouthHeight = MAX(Shape1(52, VY) - Shape1(57, VY), MouthWidth/6); // MAX is hack for smiles

FlipRectangleWithBlend(Img,
   OriginalImg,
   (Shape1(48, VX) + Shape1(54, VX)) / 2,
   (Shape1(51, VY) + Shape1(57, VY)) / 2,
   MouthWidth,
   MouthHeight,
   1.3,
   (MouthHeight/MouthWidth > .28? 2.0: 2.8), // TODO play with this
   MouthBlends, NELEMS(MouthBlends), Expand, fMirrorMouth, fShowBox);
}

//-----------------------------------------------------------------------------
static void ShiftRectangle (RgbImage &Img,  // io:
   const RgbImage &OriginalImg, // in
   int ix, int iy,              // in: bottom left of bounding rectangle (in SHAPE coords)
   int nWidth, int nHeight,     // in: size of bounding rectangle
   int nxShift,                 // in: amount to shift, +ve is rightwards
   int nyShift,                 // in: amount to shift, +ve is upwards
   double Blend = 1,            // in
   bool fShowBox = false)       // in: show rectangle borders, for debugging
{
ix += Img.width  / 2;    // convert SHAPE to RgbImage coords
iy += Img.height / 2;

ASSERT(Img.width == OriginalImg.width);
ASSERT(Img.height == OriginalImg.height);
ASSERT(Blend >= 0 && Blend <= 1);

if ((nxShift != 0 || nyShift != 0) &&
        Blend > 0 && nWidth > 0 && nHeight > 0 &&
        ix >= 0 && ix < Img.width && iy >= 0 && iy < Img.height)
    for (int j = 0; j < nWidth; j++)
        for (int i = 0; i < nHeight; i++)
            {
            const int xTo = ix + j + nxShift;
            const int yTo = iy + i + nyShift;
            const int xFrom = ix + j;
            const int yFrom = iy + i;

            if (xTo < 0 || xTo >= Img.width || xFrom < 0 || xFrom >= Img.width ||
                    yTo < 0 || yTo >= Img.height || yFrom < 0 || yFrom >= Img.height)
                continue;

            if (Blend == 1) // special case for efficiency
                Img(xTo, yTo) = OriginalImg(xFrom, yFrom);
            else
                {
                Blend(Red);
                Blend(Green);
                Blend(Blue);
                }
            if (fShowBox && (i == 0 || i == nHeight-1 || j == 0 || j == nWidth-1))
                {
                const RGB_TRIPLE Blue = {255, 0, 0};
                Img(xTo, yTo) = Blue;
                }
            }
}

//-----------------------------------------------------------------------------
// Shift the given feature by the given xShift and yShift

static void ShiftRectangleWithBlend (RgbImage &Img, // io:
   const RgbImage &OriginalImg,     // in
   double x, double y,              // in: center of rectangle
   double Width, double Height,     // in: extent of rectangle
   double WidthRatio = 1,           // in:
   double HeightRatio = 1,          // in:
   double xShift=0, double yShift=0,// in: amount to shift
   const tBlends Blends[] = NULL,   // in: NULL means use gDefaultBlends
   int nBlends = 0,                 // in: nbr of elements in Blends
   bool fShowBox = false)           // in: show rectangle borders, for debugging
{
Width   *= WidthRatio;      // width  of bounding rectangle
Height  *= HeightRatio;     // height of bounding rectangle
int ix = int(x - .5 * Width);       // bottom of bounding rectangle
int iy = int(y - .5 * Height);      // left   of bounding rectangle
int iWidth = int(Width);
int iHeight = int(Height);

if (Blends == NULL)
    {
    Blends = gDefaultBlends;
    nBlends = NELEMS(gDefaultBlends);
    }
for (int iStep = 0; iStep < nBlends; iStep++)
    {
    int iTrim = Blends[iStep].iTrim;

    ShiftRectangle(Img, OriginalImg,
                   ix + iTrim, iy + iTrim, iWidth - 2 * iTrim, iHeight - 2 * iTrim,
                   (int)xShift, (int)yShift, Blends[iStep].Blend, fShowBox && iStep == 0);
    }
}

//-----------------------------------------------------------------------------
// Randomly shift some areas of the face.
//
// This is used in (an experimental version of ) WinThatch to add
// "noise" to the image so our messing around with the eyes and mouth
// is not so obvious --- a little bit of smoke and mirrors.

static void RandomlyShiftFeatures (RgbImage &Img, // io: the face image we are going to modify
    const RgbImage &OriginalImg,    // in
    const SHAPE &Shape,             // in: landmark results of ASM search
    bool fShowBox)                  // in: show rectangle borders, for debugging
{
const tBlends Blends[] =
{
    { 0, .2 },
    { 3, .5 },
    { 5, .7 },
    { 6, 1  },
};

double InterPupilDist = Shape(36, VX) - Shape(31, VX);

ShiftRectangleWithBlend(Img,                                // lower nose
       OriginalImg,
       Shape(67, VX),
       (Shape(37, VY) + 3 * Shape(41, VY)) / 4,
       (Shape(43, VX) - Shape(39, VX)) * 1.2,
       (Shape(37, VY) - Shape(41, VY)) * .8,
       RandGauss(2), RandGauss(2),
       1, 1, Blends, NELEMS(Blends), fShowBox);

ShiftRectangleWithBlend(Img,                                // upper nose
       OriginalImg,
       (Shape(34, VX) + Shape(29, VX)) / 2,
       (Shape(34, VY) + Shape(29, VY)) / 2,
       InterPupilDist / 2.6,
       InterPupilDist / 2,
       RandGauss(2), RandGauss(2),
       1, 1, Blends, NELEMS(Blends), fShowBox);

ShiftRectangleWithBlend(Img,                                // left eyebrow
       OriginalImg,
       (Shape(24, VX) + Shape(21, VX)) / 2,
       (Shape(23, VY) + Shape(25, VY)) / 2,
       1.3 * ((Shape(24, VX) - Shape(21, VX))),
       MAX(Shape(23, VY), Shape(24, VY)) - MIN(Shape(25, VY), Shape(21, VY)) + 8,
       RandGauss(3), RandGauss(3),
       1, 1, gDefaultBlends, NELEMS(gDefaultBlends), fShowBox);

ShiftRectangleWithBlend(Img,                                // right eyebrow
       OriginalImg,
       (Shape(15, VX) + Shape(18, VX)) / 2,
       (Shape(20, VY) + Shape(22, VY)) / 2,
       1.3 * ((Shape(15, VX) - Shape(18, VX))),
       MAX(Shape(17, VY), Shape(18, VY)) - MIN(Shape(19, VY), Shape(15, VY)) + 8,
       RandGauss(3), RandGauss(3),
       1, 1, gDefaultBlends, NELEMS(gDefaultBlends), fShowBox);

ShiftRectangleWithBlend(Img,                                // forehead
       OriginalImg,
       (Shape(36, VX) + Shape(31, VX)) / 2,
       Shape(31, VY) + .7 * InterPupilDist,
       InterPupilDist * 1.4,
       InterPupilDist * .4,
       RandGauss(4), RandGauss(4),
       1, 1, gDefaultBlends, NELEMS(gDefaultBlends), fShowBox);

ShiftRectangleWithBlend(Img,                                // upper forehead
       OriginalImg,
       (Shape(36, VX) + Shape(31, VX)) / 2,
       Shape(31, VY) + 1.1 * InterPupilDist,
       InterPupilDist * 1.5,
       InterPupilDist * .5,
       RandGauss(3), RandGauss(3),
       1, 1, gDefaultBlends, NELEMS(gDefaultBlends), fShowBox);

ShiftRectangleWithBlend(Img,                                // left cheek
       OriginalImg,
       Shape(27, VX),
       Shape(27, VY) - .5 * InterPupilDist,
       InterPupilDist * .4,
       InterPupilDist * .4,
       RandGauss(4), RandGauss(4),
       1, 1, Blends, NELEMS(Blends), fShowBox);

ShiftRectangleWithBlend(Img,                                // right cheek
       OriginalImg,
       Shape(32, VX),
       Shape(32, VY) - .5 * InterPupilDist,
       InterPupilDist * .4,
       InterPupilDist * .4,
       RandGauss(4), RandGauss(4),
       1, 1, Blends, NELEMS(Blends), fShowBox);
}

//-----------------------------------------------------------------------------
void AddThirdEye (RgbImage &Img,    // io: the image with a face
  const SHAPE &Shape,               // in: landmark results of ASM search
  double Expand,                    // in: adjust flipped area size
  bool   fShowBox)                  // in: show bounding boxes of shifted areas for debugging
{
ASSERT(Shape.nrows() == 68); // Ensure XM2VTS style shape because we use hard coded-indices into Shape.
                             // For landmark numbering see e.g. Appendix A of Milborrow's masters thesis.

const RgbImage OriginalImg(Img);    // remains unchanged throughout

const tBlends Blends[] =
{
    { 0, .1 },
    { 1, .2 },
    { 2, .3 },
    { 3, .4 },
    { 4, .5 },
    { 5, .6 },
    { 6, .8 }
};

ShiftRectangleWithBlend(Img,
       OriginalImg,
       Shape(31, VX), Shape(31, VY),                // coords of left eye
       Shape(29, VX) - Shape(27, VX),               // width of left eye
       Shape(28, VY) - Shape(30, VY),               // height of left eye
       2.0,                                         // WidthRatio
       2.8,                                         // HeightRatio
       .5 * (Shape(36, VX) - Shape(31, VX)),        // amount to shift horizontally
       .5 * (Shape(36, VX) - Shape(31, VX)),        // amount to shift vertically
       Blends, NELEMS(Blends), fShowBox);
}

//-----------------------------------------------------------------------------
// Flip the image, by default vertically (but horizontally or both if you want)

void FlipImage (RgbImage &Img,           // io:
                bool fFlipHorizontal,    // in:
                bool fFlipVertical)      // in:
{
RgbImage OriginalImg(Img);

FlipRectangle(Img, OriginalImg, -Img.width/2, -Img.height/2,
              Img.width, Img.height, 1, fFlipHorizontal, fFlipVertical);
}
