// $pngtil.cpp 3.0 milbo$ PNG library hooks
//
// Based on code by Greg Roelofs in lpng1240/contrib/gregbook/readpng.c

#include "stasm.hpp"
#include "readpng.h"    /* from the PNG software distribution */

#define alpha_composite(composite, fg, alpha, bg) {               \
    ush temp = ((ush)(fg)*(ush)(alpha) +                          \
                (ush)(bg)*(ush)(255 - (ush)(alpha)) + (ush)128);  \
    (composite) = (uch)((temp + (temp >> 8)) >> 8);               \
   }

//-----------------------------------------------------------------------------
static void
CopyPngBufToRgbBuf (struct RGB_TRIPLE *pImg, // out
                    const byte *pPng, int width, int height,
                    int nRow, int nChannels,
                    byte bg_blue, byte bg_green, byte bg_red)
{
const byte *pSrc;
byte *pDest;
byte r, g, b, a;
int i;

ulg nRow4 = ((3*width + 3L) >> 2) << 2; // round up to a multiple of 4
for (int row = 0;  row < height;  ++row)
    {
    pSrc = pPng + (height - row - 1) * nRow;
    pDest = (byte *)pImg + row * nRow4;
    if (nChannels == 3)
        {
        for (i = width;  i > 0;  --i)
            {
            r = *pSrc++;
            g = *pSrc++;
            b = *pSrc++;
            *pDest++ = b;
            *pDest++ = g;   /* note reverse order */
            *pDest++ = r;
            }
        }
    else if (nChannels == 4)
        {
        for (i = width;  i > 0;  --i)
            {
            r = *pSrc++;
            g = *pSrc++;
            b = *pSrc++;
            a = *pSrc++;
            if (a == 255)
                {
                *pDest++ = b;
                *pDest++ = g;
                *pDest++ = r;
                }
            else if (a == 0)
                {
                *pDest++ = bg_blue;
                *pDest++ = bg_green;
                *pDest++ = bg_red;
                }
            else
                {
                alpha_composite(*pDest++, b, a, bg_blue);
                alpha_composite(*pDest++, g, a, bg_green);
                alpha_composite(*pDest++, r, a, bg_red);
                }
            }
        }
    else
        Err("illegal number %d of PNG channels", nChannels);
    }
}

//-----------------------------------------------------------------------------
// Load a .png file

const char *
sLoadPng (Image *pImg, RgbImage *pRgbImg,       // out
          const char sFile[], bool fExitOnErr)  // in
{
FILE *pFile = fopen(sFile, "rb");
if (!pFile)
    if (fExitOnErr)
        Err("can't open %s", sFile);
    else
        {
        sprintf(sgBuf, "Can't open %s", sFile);
        return sgBuf;
        }
ulg width, height, nRow;
int nChannels;
int rc;
if ((rc = readpng_init(pFile, &width, &height)) != 0)
    {
    switch (rc)
        {
        case 1:
            sprintf(sgBuf, "%s is not a PNG file", sFile);
            break;
        case 2:
            sprintf(sgBuf, "%s is corrupt", sFile);
            break;
        case 4:
            sprintf(sgBuf, "insufficient memory reading %s", sFile);
            break;
        default:
            sprintf(sgBuf, "readpng_init error reading %s", sFile);
            break;
        }
    if (fExitOnErr)
        Err(sgBuf);
    return sgBuf;
    }
if (width % 4)
    {
    sprintf(sgBuf, "PNG width is not divisible by 4, not yet supported"); // TODO
    if (fExitOnErr)
        Err(sgBuf);
    return sgBuf;
    }
byte bg_red, bg_green, bg_blue;
if (readpng_get_bgcolor(&bg_red, &bg_green, &bg_blue) > 1)
    {
    readpng_cleanup(true);
    sprintf(sgBuf, "readpng_get_bgcolor error reading %s", sFile);
    if (fExitOnErr)
        Err(sgBuf);
    return sgBuf;
    }
const double display_exponent = 1.0 * 2.2; // LUT_exponent * CRT_exponent
byte *pPng = readpng_get_image(display_exponent, &nChannels, &nRow);
readpng_cleanup(false);
fclose(pFile);
if (!pPng)
    {
    sprintf(sgBuf, "unable to decode %s", sFile);
    if (fExitOnErr)
        Err(sgBuf);
    return sgBuf;
    }
RgbImage RgbImg;
RgbImage *pRgb = pRgbImg;
if (!pRgb)
    pRgb = &RgbImg;
pRgb->dim(width, height);

CopyPngBufToRgbBuf(pRgb->buf, pPng, width, height,
                   nRow, nChannels, bg_red, bg_green, bg_blue);

free(pPng);

if (pImg)
    {
    // copy pRgb into pImg, converting to gray and discarding the pad

    pImg->dim(width, height);
    byte *pTo = pImg->buf;
    int nPad = width % 4; // TODO
    for (int iy = height - 1; iy >= 0; iy--)
        {
        byte *pFrom = (byte *)pRgb->buf + (3 * iy * width) + (nPad * iy);
        for (unsigned ix = 0; ix < width; ix++)
            {
            RGB_TRIPLE Rgb = *(RGB_TRIPLE *)pFrom;
            *pTo++ = (byte)RgbToGray(Rgb);  // CIE conversion to gray
            pFrom += 3;
            }
        }
    }
return NULL;    // success
}
