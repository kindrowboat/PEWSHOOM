// $rgbimutil.cpp 3.0 milbo$ image utilities
// milbo durban dec05

#include "ezfont.h"
#include "stasm.hpp"

//-----------------------------------------------------------------------------
// Helper function for ScaleRgbImage
// TODO turn this into a macro? inlining here is possibly ignored by MS compiler

static inline RGB_TRIPLE InterpolateRgbPixel (const RGB_TRIPLE *pIn,
                                        int ix, double Scale, int Max)  // in
{
int iPos = (int)(ix * Scale);
int iPos1 = iPos + 1;
if (iPos1 >= Max)   // don't overrun buffer
    iPos1 = Max-1;
RGB_TRIPLE Rgb;
for (int iColor = 0; iColor < 3; iColor++)
    {
    int c0, c1;
    switch (iColor)
        {
        case 0:     c0 = pIn[iPos].Red;     c1 = pIn[iPos1].Red;   break;
        case 1:     c0 = pIn[iPos].Green;   c1 = pIn[iPos1].Green; break;
        default:    c0 = pIn[iPos].Blue;    c1 = pIn[iPos1].Blue;  break;
        }
    double Frac = (ix * Scale) - iPos;
    double Value = (1.0 - Frac) * c0 + Frac * c1;   // linear interpolation
    byte iValue = (byte)(Value + 0.5);
    switch (iColor)
        {
        case 0:     Rgb.Red =   iValue; break;
        case 1:     Rgb.Green = iValue; break;
        default:    Rgb.Blue =  iValue; break;
        }
    }
return Rgb;
}

//-----------------------------------------------------------------------------
// Scale using two steps of linear interpolation: first in the X direction,
// then in the Y direction
//
// If fBilinear is false we use the nearest pixel (and get a sharper image)
//
// Is the following a bug? If size is reduced by more than 2, this ignores some
// pixels in the input image, even when fBilinear is true -- because we look
// a max of 2 pixels when doing bilinear interpretation.
// Use ReduceRgbImage if this matters to you.
//
// I lifted the original version of this from Henry Rowley img.cc:ReduceSize()

void ScaleRgbImage (RgbImage &Img,                  // io
                    int nNewWidth, int nNewHeight,  // in
                    bool fVerbose, bool fBilinear)  // in
{
int ix, iy;
int width = Img.width;
int height = Img.height;
double scaleX = (double)width  / nNewWidth;
double scaleY = (double)height / nNewHeight;

if (fVerbose)
    {
    if (scaleX > 1)
        lprintf("ScaleDown %.2g ", 1/scaleX);
    else if (scaleX < 1)
        lprintf("ScaleUp %.2g ", 1/scaleX);
    else
        lprintf("Scale %.2g ", 1/scaleX);
    }
if (width == nNewWidth && height == nNewHeight)
    return;                                         // NOTE: RETURN

if (!fBilinear)
    {
    // use nearest pixel

    RgbImage OutImg(nNewWidth, nNewHeight);
    for (iy = 0; iy < nNewHeight; iy++)
        for (ix = 0; ix < nNewWidth; ix++)
            OutImg(ix, iy) = Img((ix * width) / nNewWidth, (iy * height) / nNewHeight);

    Img = OutImg;
    }
else
    {
    // scale horizontally

    RgbImage Out1(height, nNewWidth);
    RGB_TRIPLE *pIn = Img.buf;
    for (iy = 0; iy < height; iy++)
        {
        for (ix = 0; ix < nNewWidth; ix++)
            Out1(iy + ix * height) = InterpolateRgbPixel(pIn, ix, scaleX, width);
        pIn += width;
        }
    // scale vertically

    Img.dim(nNewWidth, nNewHeight);
    RGB_TRIPLE *pOut = Img.buf;
    pIn = Out1.buf;

    for (ix = 0; ix < nNewWidth; ix++)
        {
        for (iy = 0; iy < nNewHeight; iy++)
            pOut[ix + iy * nNewWidth] = InterpolateRgbPixel(pIn, iy, scaleY, height);
        pIn += height;
        }
    }
}

//-----------------------------------------------------------------------------
void CropRgbImage (RgbImage &Img,                   // io
                   int nTopCrop, int nBottomCrop,   // in
                   int nLeftCrop, int nRightCrop,   // in
                   bool fWidthDivisibleBy4)         // in
{
int width = Img.width;
int height = Img.height;
if (fWidthDivisibleBy4)
    nLeftCrop += (Img.width - nRightCrop - nLeftCrop) % 4; // ensure width is div by 4
int nNewWidth = Img.width - nLeftCrop - nRightCrop;
int nNewHeight = Img.height - nTopCrop - nBottomCrop;
if (nTopCrop < 0 || nBottomCrop < 0 || nLeftCrop < 0 || nRightCrop < 0)
    Err("you can't specify a crop less than 0");
if (nNewWidth <= 0)
    Err("specified left or right crop would cause a width less than or equal to zero");
if (nNewWidth > width)
    Err("specified left or right crop would cause a width bigger than current width");
if (nNewHeight <= 0)
    Err("specified top or bottom crop would cause a height less than or equal to zero");
if (nNewHeight > height)
    Err("specified top or bottom crop would cause a height bigger than current height");

RgbImage OutImg(nNewWidth, nNewHeight);

for (int iy = 0; iy < nNewHeight; iy++)
    for (int ix = 0; ix < nNewWidth; ix++)
        OutImg(ix, iy) = Img(ix + nLeftCrop, iy + nBottomCrop);

Img = OutImg;
}

//-----------------------------------------------------------------------------
void ConvertRgbImageToGray (Image &OutImg,          // out
                            const RgbImage &Img)    // in
{
int width = Img.width;
int height = Img.height;

OutImg.dim(width, height);

for (int iy = 0; iy < height; iy++)
    for (int ix = 0; ix < width; ix++)
        OutImg(ix, iy) = (byte)RgbToGray(Img(ix + ((height - 1 - iy) * width)));
}

//-----------------------------------------------------------------------------
void DesaturateRgbImage (RgbImage &Img)  // io: convert to gray
{
int width = Img.width;
int height = Img.height;

for (int iy = 0; iy < height; iy++)
    for (int ix = 0; ix < width; ix++)
        Img(ix, iy).Red = Img(ix, iy).Green = Img(ix, iy).Blue =
            RgbToGray(Img(ix, iy));
}

//-----------------------------------------------------------------------------
#if _MSC_VER // microsoft only
static void CheckWindowsReturn (bool fSuccess, char sFile[], int iLine)
{
if (!fSuccess)
    {
    LPVOID lpMsgBuf;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf, 0, NULL);

    Err("%s %d Windows error: %s", sFile, iLine, lpMsgBuf);
    LocalFree(lpMsgBuf);
    }
}
#endif // _MSC_VER

//-----------------------------------------------------------------------------
// Convert an RgbImage to a Windows hdc, and back again.
//
// This allows you to manipulate an RgbImage using the Windows library functions.
// Not an efficient process, but useful.
//
// fFromImgToHdc=true   convert from RgbImage to hdc
//               false  convert from hdc back to RgbImage
//
// The temporary image we generate for writing has the same color characteristics
// as the screen this program is running on.  Weird, but that seems to be the only
// way under Windows.  It means if you are on a monochrome screen, for example,
// the RgbImage will be converted to monochrome.
//
// The mechanism is (in RgbImage to hdcMem direction)
//      1. Create a Windows bitmap: CreateCompatibleBitmap()
//      2. Copy RgbImage into bitmap: SetDIBits
//      3. Select the bitmap into a memory device context: SelectObject
//          Anything subsequently written into the memory device hdcMem will
//          also be written into the bitmap
//
// In the other direction, we copy the bitmap back into the RgbImage
//
// Look for "The Memory Device Context" in Petzold's book(s) for details.
// I used "Programming Windows 95" page 489.

#if _MSC_VER // microsoft only
static void ConvertRgbToWindowsHdc (RgbImage &Img, HDC &hdcMem, bool fFromImgToHdc)
{
static HDC hdc;
static HBITMAP hBmp;

static tagBITMAPINFOHEADER bmi = {
    40,     // biSize
    0,      // biWidth
    0,      // biHeight
    1,      // biPlanes
    24,     // biBitCount
    0,      // biCompression
    0,      // biSizeImage
    7085,   // biXPelsPerMeter (this value copied from an existing bitmap)
    7085,   // biYPelsPerMeter (ditto)
    0,      // biClrUsed
    0       // biClrImportant
    };
//TODO for now, silently don't write into images with width not divisible by 4
if (Img.width % 4)
    return;
if (fFromImgToHdc)
    {
    hdc = GetDC(NULL);
    hBmp = CreateCompatibleBitmap(hdc, Img.width, Img.height);
    CheckWindowsReturn((hBmp) != NULL, __FILE__, __LINE__);
    bmi.biWidth = Img.width;
    bmi.biHeight = Img.height;

    CheckWindowsReturn(SetDIBits(hdc, hBmp, 0, Img.height,
        (CONST VOID *)(Img.buf),
        LPBITMAPINFO(&bmi), DIB_RGB_COLORS) != 0, __FILE__, __LINE__);

    hdcMem = CreateCompatibleDC(NULL);
    CheckWindowsReturn(hdcMem != NULL, __FILE__, __LINE__);
    SelectObject(hdcMem, hBmp);
    }
else
    {
    DeleteDC(hdcMem);   // needed BEFORE GetDIBits (I think)
    CheckWindowsReturn(GetDIBits(hdc, hBmp, 0, Img.height,
        LPVOID(Img.buf),
        LPBITMAPINFO(&bmi), DIB_RGB_COLORS) != 0, __FILE__, __LINE__);
    ReleaseDC(NULL, hdc);
    DeleteObject(hBmp);
    }
}
#endif // _MSC_VER

//-----------------------------------------------------------------------------
// Print text into an image, at image coords ix and iy
// Use iFontSize==0 to get the default size
// Current implementation is very slow

void RgbPrintf (RgbImage &Img,                          // io
                int ix, int iy, unsigned Color,         // in
                int iFontSize, const char *pArgs, ...)  // in: args like printf
{
#if _MSC_VER // microsoft only

static bool fWarningIssued;
if (!fWarningIssued && Img.width % 4)
    {
    fWarningIssued = true;
    WarnWithNewLine("Can't print \"%s\" into image because image width "
         "is not divisible by 4", pArgs);
    return;
    }
HDC hdcMem;

ConvertRgbToWindowsHdc(Img, hdcMem, true);

va_list pArg;               // format pArgs... into s
char s[SLEN];
va_start(pArg, pArgs);
vsprintf(s, pArgs, pArg);
va_end(pArg);

HFONT      hFont;
LOGFONT    lf;

if (iFontSize == 0)
    iFontSize = 80;
hFont = EzCreateFont(hdcMem, "Courier New", iFontSize, 0, 0, true);
GetObject(hFont, sizeof(LOGFONT), &lf);
SelectObject(hdcMem, hFont);
SetBkMode(hdcMem, TRANSPARENT);
unsigned TextColor = (0xff & (Color >> 16)) |
                        (0xff00 & Color) |
                        (0xff0000 & (Color << 16));
SetTextColor(hdcMem, TextColor);
TextOut(hdcMem, ix, iy, s, strlen(s));
DeleteObject(SelectObject(hdcMem, GetStockObject(SYSTEM_FONT)));
ConvertRgbToWindowsHdc(Img, hdcMem, false);

#else

Warn("RgbPrintf is not supported on GNU builds");

// prevent warnings: unused parameter
Img = Img; ix = ix; iy = iy;
Color = Color; iFontSize = iFontSize; pArgs = pArgs;

#endif // _MSC_VER
}

//-----------------------------------------------------------------------------
// These functions are a faster version of RgbPrintf, for when
// you are printing consecutively into the same image

#if _MSC_VER // these function are only supported under Windows

static HDC hdcgMem;

void BeginRgbPrintf (RgbImage &Img)
{
ASSERT(hdcgMem == NULL);
if (Img.width % 4)
    WarnWithNewLine("Can't print into image because image width %d "
         "is not divisible by 4", Img.width);
else
    ConvertRgbToWindowsHdc(Img, hdcgMem, true);
}

void DoRgbPrintf (int ix, int iy,
                  unsigned Color, int iFontSize,
                  const char *pArgs, ...)   // in: args like printf
{
if (!hdcgMem)   // BeginRgbPrintf failed?
    return;

va_list pArg;   // format pArgs... into s
char s[SLEN];
va_start(pArg, pArgs);
vsprintf(s, pArgs, pArg);
va_end(pArg);

if (iFontSize == 0)
    iFontSize = 80;
HFONT hFont = EzCreateFont(hdcgMem, "Courier New", iFontSize, 0, 0, true);
LOGFONT lf;
GetObject(hFont, sizeof(LOGFONT), &lf);
SelectObject(hdcgMem, hFont);
SetBkMode(hdcgMem, TRANSPARENT);
unsigned TextColor = (0xff & (Color >> 16)) |
                     (0xff00 & Color) |
                     (0xff0000 & (Color << 16));
SetTextColor(hdcgMem, TextColor);
TextOut(hdcgMem, ix, iy, s, strlen(s));
DeleteObject(SelectObject(hdcgMem, GetStockObject(SYSTEM_FONT)));
}

void EndRgbPrintf (RgbImage &Img)
{
if (!hdcgMem)       // BeginRgbPrintf failed?
    return;
ConvertRgbToWindowsHdc(Img, hdcgMem, false);
hdcgMem = NULL;

}
#endif // _MSC_VER
