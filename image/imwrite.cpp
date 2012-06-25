// $imwrite.cpp 3.0 milbo$ routines for writing BMP files
//
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

// following copied from windows.h (and allows us to compile
// in non WIN32 environments)

typedef unsigned short WORD;
typedef unsigned int   DWORD;
typedef int LONG;

#pragma pack(2)

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

//-----------------------------------------------------------------------------
static void
WriteBmpHeader (FILE *pBmpFile, char sFile[], int width, int height)    // in: all
{
BITMAPFILEHEADER    bmfHeader;
BITMAPINFOHEADER    bmiHeader;

ASSERT(width > 0 && height > 0);
int nPad = width % 4;
int nNewWidth = width * 3 + nPad;

bmfHeader.bfType        = ((WORD) ('M' << 8) | 'B');
bmfHeader.bfSize        = (height * nNewWidth) + 56;
bmfHeader.bfReserved1   = 0;
bmfHeader.bfReserved2   = 0;
bmfHeader.bfOffBits     = 54;

Fwrite(&bmfHeader, 1, sizeof(bmfHeader), pBmpFile, sFile);

bmiHeader.biSize            = 40;       // sizeof(bmiHeader) always 40
bmiHeader.biWidth           = width;    // width
bmiHeader.biHeight          = height;   // height
bmiHeader.biPlanes          = 1;        // 1    These numbers are for RGB
bmiHeader.biBitCount        = 24;       // 24   24 bit uncompressed files
bmiHeader.biCompression     = 0;        // 0 no compression
bmiHeader.biSizeImage       = 0;        // 0
bmiHeader.biXPelsPerMeter   = 2834;     // 2834 magic number copied from existing bitmaps
bmiHeader.biYPelsPerMeter   = 2834;     // 2834 ditto
bmiHeader.biClrUsed         = 0;        // 0 no color map entries
bmiHeader.biClrImportant    = 0;        // 0

Fwrite(&bmiHeader, 1, sizeof(bmiHeader), pBmpFile, sFile);
}

//-----------------------------------------------------------------------------
static void
WriteBmpPixels (FILE *pBmpFile, char sFile[],           // in: all
                byte *buf, int width, int height)
{
int nPad = width % 4;
int Pixel;
for (int iy = height - 1; iy >= 0; iy--)
    {
    for (int ix = 0; ix < width; ix++)
        {
        Pixel = *(buf + ix + iy * width);
        Pixel = (Pixel | (Pixel << 8) | (Pixel << 16));     // gray to RGB
        Fwrite(&Pixel, 1, 3, pBmpFile, sFile);
        }
    if (nPad)
        {
        Pixel = 0;
        Fwrite(&Pixel, 1, nPad, pBmpFile, sFile);
        }
    }
Pixel = 0;
Fwrite(&Pixel, 1, 2, pBmpFile, sFile);  // photoshop puts 2 bytes on end, so we do too
}

//-----------------------------------------------------------------------------
static void
WriteBmpPixels (FILE *pBmpFile, char sFile[],           // in: all
                RGB_TRIPLE *buf, int width, int height)
{
int nPad = width % 4;
int PadPixel = 0;
for (int iy = 0; iy < height; iy++)
    {
    Fwrite(buf + iy * width, 1, 3 * width, pBmpFile, sFile);
    if (nPad)
        Fwrite(&PadPixel, 1, nPad, pBmpFile, sFile);
    }
Fwrite(&PadPixel, 1, 2, pBmpFile, sFile);
}

//-----------------------------------------------------------------------------
// Write the given buffer as a 24 bit uncompressed windows BMP file
//
// A bitmap file looks like this:
//
//    BITMAPFILEHEADER bmfHeader;
//    BITMAPINFOHEADER bmiHeader;   <---hDib points to start of this
//    RGBTRIPLE        Colors[]
//
//  where (values given in comments are for windows 24 bit uncompressed RGB files)
//
//  typedef struct tagBITMAPFILEHEADER {
//        WORD    bfType;               // always "BM"
//        DWORD   bfSize;               // (height * width * 3) + padding (see below)
//        WORD    bfReserved1;          // always 0
//        WORD    bfReserved2;          // always 0
//        DWORD   bfOffBits;            // always 54 (24bit uncompressed BMPs)
// } BITMAPFILEHEADER, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
//
// typedef struct tagBITMAPINFOHEADER {
//        DWORD      biSize;            // sizeof(bmiHeader) always 40
//        LONG       biWidth;           // width
//        LONG       biHeight;          // height
//        WORD       biPlanes;          // 1    These numbers are for RGB
//        WORD       biBitCount;        // 24   24 bit uncompressed files
//        DWORD      biCompression;     // 0    uncompressed data
//        DWORD      biSizeImage;       // 0 usually
//        LONG       biXPelsPerMeter;   // magic number copied from existing bitmaps
//        LONG       biYPelsPerMeter;   // ditto (doesn't matter in this application)
//        DWORD      biClrUsed;         // 0
//        DWORD      biClrImportant;    // 0
// } BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

void
WriteBmp (const Image &Img, const char sFile[], bool fVerbose)  // in: all
{
FILE    *pBmpFile;
char    sDrive[_MAX_DRIVE], sDir[_MAX_DIR], sBase[_MAX_FNAME], sExt[_MAX_EXT];
char    sFile1[SLEN];
int width = Img.width, height = Img.height;

splitpath(sFile, sDrive, sDir, sBase, sExt); // make sure file has a .bmp extension

// if user has strange extension don't silently overwrite it (4 is ".bmp")
ASSERT(strlen(sExt) <= 4);

makepath(sFile1, sDrive, sDir, sBase, ".bmp");

if (fVerbose)
    lprintf("Writing %s %dx%d\n", sFile1, width, height);

pBmpFile = Fopen(sFile1, "wb"); // will invoke Err if can't open
if (pBmpFile)                   // succesful open?
    {
    WriteBmpHeader(pBmpFile, sFile1, width, height);
    WriteBmpPixels(pBmpFile, sFile1, Img.buf, width, height);
    fclose(pBmpFile);
    }
}

//-----------------------------------------------------------------------------
void
WriteBmp (const RgbImage &Img, const char sFile[], bool fVerbose)   // in: all
{
FILE    *pBmpFile;
char    sDrive[_MAX_DRIVE], sDir[_MAX_DIR], sBase[_MAX_FNAME], sExt[_MAX_EXT];
char    sFile1[SLEN];
int width = Img.width, height = Img.height;

splitpath(sFile, sDrive, sDir, sBase, sExt);  // make sure file has a .bmp extension

// if user has strange extension don't silently overwrite it (4 is ".bmp")
ASSERT(strlen(sExt) <= 4);

makepath(sFile1, sDrive, sDir, sBase, ".bmp");

if (fVerbose)
    lprintf("Writing %s %dx%d\n", sFile1, width, height);

pBmpFile = Fopen(sFile1, "wb");
WriteBmpHeader(pBmpFile, sFile1, width, height);
WriteBmpPixels(pBmpFile, sFile1, Img.buf, width, height);
fclose(pBmpFile);
}
