// $imequalize.cpp 3.0 milbo$ image equalization and related routines

#include "stasm.hpp"

// only look at every CONF_nSkipDist'th pixel, for quick equalization
#define CONF_nSkipDist 8

//-----------------------------------------------------------------------------
// Unlike EqualizeImageWithLightingCorrection this can deal with a
// non-even mask width or height.

void EqualizeImage (Image &Img,     // io:
                    bool fQuick)    // in: if true do a quick and dirty equalize
{
// compute the histogram

int Hist[256];
memset(Hist, 0, sizeof(Hist));
int i;
for (i = 0; i < Img.width * Img.height; i += (fQuick? CONF_nSkipDist: 1))
    Hist[Img(i)]++;

// compute cumulative histogram

int CumHist[256];
int Sum = 0;
for (i = 0; i < 256; i++)
    {
    CumHist[i] = Sum;
    Sum += Hist[i];
    }
int Total = Sum;
for (i = 255; i >= 0; i--)
    {
    CumHist[i] += Sum;
    Sum -= Hist[i];
    }
// copy back into image, applying histogram equalization

for (i = 0; i < Img.width * Img.height; i++)
    Img(i) = (byte)((127 * CumHist[Img(i)]) / Total);
}

//-----------------------------------------------------------------------------
// Like EqualizeImage but mirrors the image too
// This is separate from EqualizeImage for efficiency (no fMirrow check
// needed in the loops)

void EqualizeMirrorImage (Image &Img)
{
int     iPixel = 0;
int     i, j;
int     *pTempImg = new int[Img.width * Img.height];

// compute the histogram and copy mirrored image into pTmpImage

int Hist[256];
memset(Hist, 0, sizeof(Hist));
iPixel = 0;
for (j = 0; j < Img.height; j++)
    {
    int jWidth = j * Img.width;
    for (i = Img.width - 1; i >= 0; i--)
        {
        int Pixel = Img(i +  jWidth);
        pTempImg[iPixel++] = Pixel;
        Hist[Pixel]++;
        }
    }
// compute cumulative histogram

int CumHist[256];
int Sum = 0;
for (i = 0; i < 256; i++)
    {
    CumHist[i] = Sum;
    Sum += Hist[i];
    }
int Total = Sum;
for (i = 255; i >= 0; i--)
    {
    CumHist[i] += Sum;
    Sum -= Hist[i];
    }
// copy back into image, applying histogram equalization

for (i = 0; i < Img.width * Img.height; i++)
    Img(i) = (byte)((127 * CumHist[pTempImg[i]]) / Total);

delete[] pTempImg;
}

//-----------------------------------------------------------------------------
// Like EqualizeImage but for RGB images

void EqualizeRgbImage (RgbImage &Img)
{
// compute the histogram

int Hist[256];
memset(Hist, 0, sizeof(Hist));
int i;
for (i = 0; i < Img.width * Img.height; i++)
    Hist[RgbToGray(Img(i))]++;

// compute cumulative histogram

int CumHist[256];
int Sum = 0;
for (i = 0; i < 256; i++)
    {
    CumHist[i] = Sum;
    Sum += Hist[i];
    }
int Total = Sum;
for (i = 255; i >= 0; i--)
    {
    CumHist[i] += Sum;
    Sum -= Hist[i];
    }
// copy back into image, applying histogram equalization

Total *= 2;
for (i = 0; i < Img.width * Img.height; i++)
    {
    Img(i).Red   = (byte)((Img(i).Red   * CumHist[Img(i).Red])   / Total);
    Img(i).Green = (byte)((Img(i).Green * CumHist[Img(i).Green]) / Total);
    Img(i).Blue  = (byte)((Img(i).Blue  * CumHist[Img(i).Blue])  / Total);
    }
}

//-----------------------------------------------------------------------------
// A quick version of EqualizeRgbImage.  Currently used only by marki.
//
// This decides what equalization to do by only looking at the Green pixels.
// Suitable for quick equalizing of gray images that are stored as RGB images
// Why Green? Because it gets the most weighting in the CIE conversion from gray to color.

void QuickEqualizeRgbImage (RgbImage &Img,    // io
                            double EqAmount)  // in: 0 to 1, amount of equalization
{
// compute the histogram, look at only every CONF_nSkipDist'th pixel to save time

int Hist[256];
memset(Hist, 0, sizeof(Hist));
int i;
for (i = 0; i < Img.width * Img.height; i += CONF_nSkipDist)
    Hist[Img(i).Green]++;

// compute cumulative histogram

double CumHist[256];
double Sum = 0;
for (i = 0; i < 256; i++)
    {
    CumHist[i] = Sum;
    Sum += Hist[i];
    }
double Sum2 = Sum * 2;
for (i = 255; i >= 0; i--)
    {
    CumHist[i] += Sum;
    Sum -= Hist[i];
    }

// scale CumHist for EqAmount

if (EqAmount < 0 || EqAmount > 1)
    Err("EqAmount %g is out of range, valid range is 0 to 1", EqAmount);
const double temp = 3 - 3 * sqrt(EqAmount);
for (i = 255; i >= 0; i--)
    {
    CumHist[i] /= Sum2;
    CumHist[i] = (temp + CumHist[i]) / (temp + 1);
    }

// copy back into image, applying histogram equalization

for (i = 0; i < Img.width * Img.height; i++)
    {
    Img(i).Red   = (byte)(Img(i).Red   * CumHist[Img(i).Red]);
    Img(i).Green = (byte)(Img(i).Green * CumHist[Img(i).Green]);
    Img(i).Blue  = (byte)(Img(i).Blue  * CumHist[Img(i).Blue]);
    }
}

//-----------------------------------------------------------------------------
static void GetImageCumHist (int CumHist[], // out: declare as: int CumHist[256]
              int &Total,                                   // out:
              const Image &Img,                             // in:
              int nxMin, int nxMax, int nyMin, int nyMax)   // in: SHAPE coords
{
int i, j;

nxMin += Img.width/2;       // shape coord to image coords
nxMax += Img.width/2;
nyMin += Img.height/2;
nyMax += Img.height/2;

ASSERT(nxMin >= 0 && nxMin <= nxMax);
ASSERT(nyMin >= 0 && nyMin <= nyMax);
ASSERT(nxMax >= 0 && nxMax <= Img.width);
ASSERT(nyMax >= 0 && nyMax <= Img.height);

// compute the histogram

int Hist[256];
memset(Hist, 0, sizeof(Hist));
for (j = nxMin; j < nxMax; j++)
    for (i = nyMin; i < nyMax; i++)
        Hist[Img(j,i)]++;

// compute cumulative histogram

int Sum = 0;
for (i = 0; i < 256; i++)
    {
    CumHist[i] = Sum;
    Sum += Hist[i];
    }
Total = Sum;
for (i = 255; i >= 0; i--)
    {
    CumHist[i] += Sum;
    Sum -= Hist[i];
    }
}

//-----------------------------------------------------------------------------
// Apply histogram equalization to Img

static void ApplyImageCumHist (Image &Img,      // io
                        const int CumHist[],    // in: declare as: int CumHist[256]
                        int Total)              // in
{
for (int i = 0; i < Img.width * Img.height; i++)
    Img(i) = (byte)((127 * CumHist[Img(i)]) / Total);
}

//-----------------------------------------------------------------------------
// This does histogram equalization on an image as per
// Henry Rowley's PhD Thesis section 2.5, and this code is lifted from his code.
//
// This does not yet do lighting correction, so the "Correct" part of the
// name is not yet implemented.
//
// It uses pixels in window specified by nxMin nxMax nyMin nyMax to determine
// equalization parameters then equalizes the entire image with these parameters.
//
// About histogram equalization
// ----------------------------
//
// The goal is to produce an image with a flat histogram i.e.
// an image where each pixel appear the same number number of
// times.  The cumulative histogram of such an image will have
// the property that the number of pixels with an intensity
// less than or equal to a given intensity is proportional to
// that intensity. In practice it is impossible to get a
// perfectly flat histogram (for example, for an input image
// with a constant intensity), so the result is only an
// approximately flat histogram.  (This is a more-or-less a
// quote from section 2.5 of Rowley's thesis).

void EqualizeAndCorrectImage (Image &Img,                   // io:
                int nxMin, int nxMax, int nyMin, int nyMax, // in: mask area
                bool fVerbose)                              // in
{
if (fVerbose)
    lprintf("equalize ");

int CumHist[256];
int Total;
GetImageCumHist(CumHist, Total, Img, nxMin, nxMax, nyMin, nyMax);
ApplyImageCumHist(Img, CumHist, Total);
}
