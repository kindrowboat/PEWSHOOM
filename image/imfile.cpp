// $imfile.cpp 3.1 milbo$ routines for image files
//
// Many routines in this file use the general purpose global
// buffer sgBuf for storing error messages.
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
// Load a .bmp file.  See the header of sLoadImage below for some details.

static char *
sLoadBmp (Image *pImg, RgbImage *pRgbImg,                           // out
        const char sFile[], bool fExitOnErr, bool fRemovePad=true,  // in
        bool fDeleteOnErr=false)                                    // in
{
BITMAPFILEHEADER    bmfHeader;
BITMAPINFOHEADER    bmiHeader;
FILE                *pBmpFile;

ASSERT(sizeof(WORD) == 2);
ASSERT(sizeof(DWORD) == 4);
ASSERT(sizeof(LONG) == 4);
// if these fail, fix the packing in BITMAPFILEHEADER and BITMAPINFOHEADER above
ASSERT(sizeof(bmfHeader) == 14);
ASSERT(sizeof(bmiHeader) == 40);

int width = 0;
int height = 0;
unsigned Len = 0;
int nPad = 0;
sgBuf[0] = 0;   // will put error messages into here, if any
pBmpFile = fopen(sFile, "rb");
if (!pBmpFile)
    sprintf(sgBuf, "Can't open %s", sFile);

if (!sgBuf[0] &&
        ((Len = fread(&bmfHeader, 1, sizeof(bmfHeader), pBmpFile)) != sizeof(bmfHeader) ||
            fread(&bmiHeader, 1, sizeof(bmiHeader), pBmpFile) != sizeof(bmiHeader)))
    sprintf(sgBuf, "Can't read %s%s", sFile, (Len? " (too short to be a BMP file)": ""));

if (!sgBuf[0] && (bmfHeader.bfType != ((WORD) ('M' << 8) | 'B')))
    sprintf(sgBuf, "%s is not a BMP file (first two bytes are not \"BM\")", sFile);

if (!sgBuf[0] &&
        (bmiHeader.biBitCount != 24 || bmiHeader.biClrUsed || bmiHeader.biCompression))
    {
    sprintf(sgBuf, "%s is not a windows 24 bit uncompressed RGB file ", sFile);
    if (bmiHeader.biBitCount!=24)
        strcat(sgBuf, "(bit count is not 24)");
    else if (bmiHeader.biClrUsed)
        strcat(sgBuf, "(ClrUsed is set)");
    else if (bmiHeader.biCompression)
        strcat(sgBuf, "(file is compressed)");
    }
if (!sgBuf[0])
    {
    width = bmiHeader.biWidth;
    height = bmiHeader.biHeight;
    if (width < 1 || width > MAX_IMAGE_DIM || height > MAX_IMAGE_DIM || height < 1)
        sprintf(sgBuf, "%s has a strange width %d or height %d", sFile, width, height);
    }
byte *pRgbTemp = NULL;
nPad = width % 4;
bool fStoreDataInRgbImageBuf = (pRgbImg && nPad == 0);
if (!sgBuf[0])
    {
    Len = (((3 * width) + nPad) * height);
    if (fStoreDataInRgbImageBuf)
        {
        // use image buffer directly, for efficiency
        pRgbImg->dim(width + (fRemovePad? 0: nPad), height);
        pRgbTemp = (byte *)pRgbImg->buf;
        fStoreDataInRgbImageBuf = true;
        }
    else  // have to use a temporary buffer
        pRgbTemp = (byte *)malloc(Len);
    if (fread(pRgbTemp, 1, Len, pBmpFile) != Len)
        {
        if (fStoreDataInRgbImageBuf)
            pRgbImg->free();
        else
            free(pRgbTemp);
        sprintf(sgBuf, "Can't read %s (it's shorter than its header says it should be)",
            sFile);
        }
    }
if (pBmpFile)
    fclose(pBmpFile);
if (sgBuf[0])
    {
    if (fDeleteOnErr)
        unlink(sFile);
    if (fExitOnErr)
        Err(sgBuf);
    else
        return sgBuf;
    }
if (pImg)
    {
    // copy pRgbTemp into pImg, converting to gray and discarding the pad

    pImg->dim(width, height);
    byte *pTo = pImg->buf;
    for (int iy = height - 1; iy >= 0; iy--)
        {
        byte *pFrom = pRgbTemp + (3 * iy * width) + (nPad * iy);
        for (int ix = 0; ix < width; ix++)
            {
            RGB_TRIPLE Rgb = *(RGB_TRIPLE *)pFrom;
            *pTo++ = (byte)RgbToGray(Rgb);  // CIE conversion to gray
            pFrom += 3;
            }
        }
    }
if (!fStoreDataInRgbImageBuf)
    {
    if (pRgbImg)
        {
        // copy pRgbTemp into pRgbImg, discarding the nasty little pad

        pRgbImg->dim(width + (fRemovePad? 0: nPad), height);
        RGB_TRIPLE *pTo = pRgbImg->buf;
        RGB_TRIPLE *pFrom = (RGB_TRIPLE *)pRgbTemp;
        for (int iy = 0; iy < height; iy++)
            {
            for (int ix = 0; ix < width; ix++)
                *pTo++ = *pFrom++;
            byte *r = (byte *)pFrom;
            r += nPad;
            pFrom = (RGB_TRIPLE *)r;
            }
        }
    free(pRgbTemp);
    }
return NULL;    // success
}

//-----------------------------------------------------------------------------
// Load a .pgm file.  See the header of sLoadImage for some details.

static const char *
sLoadPgm (Image *pImg, RgbImage *pRgbImg,       // out
           const char sFile[], bool fExitOnErr) // in
{
FILE *pFile = fopen(sFile, "rb");
if (!pFile)
    {
    if (fExitOnErr)
        Err("can't open %s", sFile);
    else
        {
        sprintf(sgBuf, "Can't open %s", sFile);
        return sgBuf;
        }
    }
int width = -1, height = -1, DataSize, nTokens = 0;
do
    {
    char sLine[SLEN];
    (void)fgets(sLine, SLEN, pFile);
    char *comment = strchr(sLine,'#');
    if (comment != NULL)
        *comment = 0;
    char *token = NULL, *input = &sLine[0];
    while (nTokens < 4 && (token = strtok(input," \t\n\r")) != NULL)
        {
        switch (nTokens)
            {
            case 0:
                if (strcmp(token,"P5") != 0)
                    {
                    if (fExitOnErr)
                        Err("%s has a bad header (first two bytes are not \"P5\")", sFile);
                    else
                        return "bad header (first two bytes are not \"P5\")";
                    }
                break;
            case 1:
                width = atoi(token);
                break;
            case 2:
                height = atoi(token);
                break;
            case 3:
                DataSize = atoi(token);
                if (DataSize != 255)
                    {
                    if (fExitOnErr)
                        Err("this program can only deal with data sizes of 255 and "
                            "%s has a data size of %d", sFile, DataSize);
                    else
                        return "data size not 255";
                    }
                break;
            default:
                break;
            }
        nTokens++;
        input = NULL;
        }
    }
while (nTokens < 4);

if (width < 1 || width > MAX_IMAGE_DIM || height > MAX_IMAGE_DIM || height < 1)
    {
    if (fExitOnErr)
        Err("%s has a strange width %d or height %d", sFile, width, height);
    else
        return "strange width or height";
    }
Image ImgTemp(width, height);
Fread((char *)ImgTemp.buf , 1, width * height, pFile, sFile);
fclose(pFile);

if (pRgbImg)
    {
    pRgbImg->dim(width, height);
    byte *pTo = (byte *)(pRgbImg->buf);
    for (int iy = height-1; iy >= 0; iy--)
        for (int ix = 0; ix < width; ix++)
            {
            byte b = ImgTemp(ix, iy);
            *pTo++ = b;
            *pTo++ = b;
            *pTo++ = b;
            }
    }
if (pImg)
    *pImg = ImgTemp;

return NULL;    // success
}

//-----------------------------------------------------------------------------
const char *
sLoadPgm (Image &Img,                           // out
           const char sFile[], bool fExitOnErr) // in
{
return sLoadPgm(&Img, NULL, sFile, fExitOnErr);
}

//-----------------------------------------------------------------------------
// Load a .ppm file.  See the header of sLoadImage for some details.

static const char *
sLoadPpm (Image *pImg, RgbImage *pRgbImg, const char sFile[], bool fExitOnErr)
{
FILE *pFile = fopen(sFile, "rb");
if (!pFile)
    {
    if (fExitOnErr)
        Err("can't open %s", sFile);
    else
        {
        sprintf(sgBuf, "can't open %s", sFile);
        return sgBuf;
        }
    }
int width = -1, height = -1, DataSize, nTokens = 0;
do
    {
    char sLine[SLEN];
    (void)fgets(sLine, SLEN, pFile);
    char *comment = strchr(sLine,'#');
    if (comment != NULL)
        *comment = 0;
    char *token = NULL, *input = &sLine[0];
    while (nTokens < 4 && (token = strtok(input," \t\n\r")) != NULL)
        {
        switch (nTokens)
            {
            case 0:
                if (strcmp(token,"P6") != 0)
                    {
                    if (fExitOnErr)
                        Err("%s has a bad header (first two bytes are not \"P6\")", sFile);
                    else
                        return "bad header (first two bytes are not \"P6\")";
                    }
                break;
            case 1:
                width = atoi(token);
                break;
            case 2:
                height = atoi(token);
                break;
            case 3:
                DataSize = atoi(token);
                if (DataSize != 255)
                    {
                    if (fExitOnErr)
                        Err("this program can only deal with data sizes of 255\n"
                            "       and %s has a data size of %d", sFile, DataSize);
                    else
                        return "data size not 255";
                    }
                break;
            default:
                break;
            }
        nTokens++;
        input = NULL;
        }
    }
while (nTokens < 4);
if (width < 1 || width > MAX_IMAGE_DIM || height > MAX_IMAGE_DIM || height < 1)
    {
    if (fExitOnErr)
        Err("%s has a strange width %d or height", sFile, width, height);
    else
        return "strange width or height";
    }
RgbImage RgbTemp(width, height);
Fread(RgbTemp.buf, 1, 3 * width * height, pFile, sFile);
fclose(pFile);

if (pImg)
    {
    // copy RgbTemp into buf, converting to gray

    pImg->dim(width, height);
    for (int iy = 0; iy < height; iy++)
        for (int ix = 0; ix < width; ix++)
            {
            // CIE conversion to gray, add 500 to take care of rounding.
            // Colors are reversed so have to take care of that too.
            RGB_TRIPLE c = RgbTemp(ix, iy);
            (*pImg)(ix, iy) = (byte)
                ((299 * c.Blue + 587 * c.Green + 114 * c.Red + 500)/1000);
            }
    }
if (pRgbImg)
    {
    // Image comes in upside down with colors reversed.  Fix that.

    pRgbImg->dim(width, height);
    for (int iy = 0, y1 = height-1; iy < height; iy++, y1--)
        for (int ix = 0; ix < width; ix++)
            {
            (*pRgbImg)(ix, iy).Red = RgbTemp(ix, y1).Blue;
            (*pRgbImg)(ix, iy).Green = RgbTemp(ix, y1).Green;
            (*pRgbImg)(ix, iy).Blue = RgbTemp(ix, y1).Red;
            }
    }
return NULL;    // success
}

//-----------------------------------------------------------------------------
static FILE *
pOpenTempFile (char sFile[],                                    // out
                const char sTemplate[], const char sMode[],     // in
                const char sOptionalMsg[] = NULL)
{
makepath(sFile, "", sGetTempDir(), "stasm-temp", "bmp");

return Fopen(sFile, sMode, sOptionalMsg); // will give err msg and exit if can't open
}

//-----------------------------------------------------------------------------
// Load an image.  Knows about different types of files like BMPs and JPEGs
//
// Returns an error message if necessary, else NULL.
//
// This uses the file extension to determine the file type.
//
// pImg is gray version of the image.      Caller must free.  Set to NULL if don't need.
// pRgbImg is color version of the image.  Caller must free.  Set to NULL if don't need.
//
// sFile is the filename.
// Set fVerbose if you want to tell the user that you are reading the file.
//
// fRemovePad=false only applies to RGB bitmaps and is needed
// for StretchDIBits() under windows

const char *
sLoadImage (Image *pImg, RgbImage *pRgbImg,                                      // out
            const char sFile[], bool fVerbose, bool fExitOnErr, bool fRemovePad) // in
{
if (fVerbose)
    lprintf("Reading %s\n", sFile);

if (sFile[0] == 0)
    {
    sprintf(sgBuf, "Null path name, can't load image");
    if (fExitOnErr)
        Err(sgBuf);
    else
        return sgBuf;
    }
char sBase[_MAX_FNAME], sExt[_MAX_EXT];
splitpath(sFile, NULL, NULL, sBase, sExt);

if (sExt[0] == 0)
    {
    sprintf(sgBuf,
        "Image file %s has no extension: can't determine the image type to read from disk",
        sFile);
    if (fExitOnErr)
        Err(sgBuf);
    else
        return sgBuf;
    }
switch (toupper(sExt[1]))
    {
    case 'B':
        return sLoadBmp(pImg, pRgbImg, sFile, fExitOnErr, fRemovePad);
        break;
    case 'J':
        {
        // File is a JPEG file: create a temporary BMP file and use that instead.
        // TODO this isn't an efficient way of doing it.
        // TODO you can't reliably run stasm or tasm at the same time
        //      as marki with this (or marki twice)

        char sTempPath[SLEN];
        FILE *pTempFile = pOpenTempFile(sTempPath, sBase, "w+b",
                                        "temporary file for converting JPG to BMP");
        char *s = sConvertJpgFileToBmpFile(sFile, pTempFile, 0);
        fclose(pTempFile);
        if (s == NULL)
            s = sLoadBmp(pImg, pRgbImg, sTempPath, fExitOnErr, true /* fDeleteOnErr */);
        unlink(sTempPath);
        return s;
        }
        break;
    case 'P':
        if (toupper(sExt[2]) == 'N')
#if _MSC_VER
            return sLoadPng(pImg, pRgbImg, sFile, fExitOnErr);
#else
            Err("PNG files are not supported in this version");
#endif
        else if (toupper(sExt[2]) == 'G')
            return sLoadPgm(pImg, pRgbImg, sFile, fExitOnErr);
        else
            return sLoadPpm(pImg, pRgbImg, sFile, fExitOnErr);
        break;
    default:
        {
        // open merely so we can give a decent error message
        FILE *pFile = fopen(sFile, "rb");
        if (!pFile)
            {
            sprintf(sgBuf, "Can't open %s", sFile);
            if (fExitOnErr)
                Err(sgBuf);
            else
                return sgBuf;
            }
        fclose(pFile);
        sprintf(sgBuf, "%s is not a recognized image type", sFile);
        if (fExitOnErr)
            Err(sgBuf);
        else
            return sgBuf;
        break;
        }
    }
return NULL;    // success
}

//-----------------------------------------------------------------------------
// Load an image.  Knows about different types of files like BMPs and JPEGs.
// This uses the file extension to determine file type.
//
// Img is a gray image
// sFile is the filename
// Set fVerbose if you want to tell the user that you are reading the file

const char *
sLoadImage (Image &Img,                                           // out
            const char sFile[], bool fVerbose, bool fExitOnErr)   // in
{
return sLoadImage(&Img, NULL, sFile, fVerbose, fExitOnErr, true);
}

//-----------------------------------------------------------------------------
// See above header.  RgbImg is a color image.
// fRemovePad=false is needed to for StretchDIBits() under windows

const char *
sLoadImage (RgbImage &RgbImg,                                                    // out
            const char sFile[], bool fVerbose, bool fExitOnErr, bool fRemovePad) // in
{
return sLoadImage(NULL, &RgbImg, sFile, fVerbose, fExitOnErr, fRemovePad);
}
