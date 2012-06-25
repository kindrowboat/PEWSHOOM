// $thatch.cpp 3.0 milbo$ generate thatcher faces using the stasm library
//
// milbo petaluma apr 2010

#include "stasm.hpp"
#include "../thatch/libthatch.hpp"

#define DRAW_LANDMARKS       0
#define WRITE_ORIGINAL_IMAGE 1
#define WRITE_FLIPPED_IMAGE  1

//-----------------------------------------------------------------------------
int main (int argc, const char *argv[])
{
if (argc < 2)
    {
    lprintf("Usage: thatch IMAGE1 [IMAGE2 ...]\n");
    exit(-1);
    }
const char **psImage = argv + 1;    // pointer to image filename

lprintf("thatch version 1.0 based on stasm %s\n", STASM_VERSION);

char sDataDir[SLEN];        // data dir (for face detector files etc.)
GetDataDir(sDataDir, *argv);

char sConfFile0[SLEN], sConfFile1[SLEN]; // configuration files to specify the ASM models
sprintf(sConfFile0, "%s/%s", sDataDir, "mu-68-1d.conf");
sprintf(sConfFile1, "%s/%s", sDataDir, "mu-76-2d.conf");

while (--argc)
    {
    lprintf("%s: ", *psImage);

    if (**psImage == '-')
        {
        // user supplied a command line argument, that's not allowed
        lprintf("illegal flag\n\nUsage: thatch IMAGE1 [IMAGE2 ...]\n");
        exit(-1);
        }
    // read the image from disk (will issue error and exit if can't)

    RgbImage Img; sLoadImage(Img, *psImage);

    // find the facial landmarks, put them into Shape

    SHAPE StartShape;       // dummy arg for AsmSearch
    DET_PARAMS DetParams;   // dummy arg for AsmSearch
    double MeanTime;        // dummy arg for AsmSearch

    SHAPE Shape = AsmSearch(StartShape, DetParams, MeanTime,
                            Img, *psImage, false,
                            sConfFile0, sConfFile1, sDataDir);

    if (Shape.nrows())      // AsmSearch successful?
        {
        char sFilename[SLEN];

#if DRAW_LANDMARKS
       // draw face detector box and landmarks on the image

        DrawShape(Img, DetParamsToShape(DetParams), C_YELLOW, true, C_YELLOW);
        DrawShape(Img, Shape);
#endif
        RgbImage ImgCopy(Img);

#if WRITE_ORIGINAL_IMAGE
        // write original image to disk for easy comparison (but crop it to the face)

        CropImageToShape(ImgCopy, Shape);
        #if _MSC_VER // Microsoft (because RgbPrintf not yet supported under gcc)
        RgbPrintf(ImgCopy, 5, 2, C_CYAN, 120, "original");
        #endif
        sprintf(sFilename, "th-%s-org.bmp", sGetBase(*psImage));
        lprintf("%s ", sFilename);
        WriteBmp(ImgCopy, sFilename);
#endif
        // thatcherize the face in the image and write to disk

        SeedRand(99); // SeedRand for repeatability when same filename twice on command line
        Thatcherize(Img, Shape);
        CropImageToShape(Img, Shape);
        ImgCopy = Img; // "=" is overloaded for RgbImage, so this does a memcpy
        #if _MSC_VER
        RgbPrintf(ImgCopy, 5, 2, C_CYAN, 120, "thatch");
        #endif
        sprintf(sFilename, "th-%s.bmp", sGetBase(*psImage));
        lprintf("%s ", sFilename);
        WriteBmp(ImgCopy, sFilename);

#if WRITE_FLIPPED_IMAGE
        // write thatcherized image, but upside-down

        FlipImage(Img);
        #if _MSC_VER
        RgbPrintf(Img, 5, 2, C_CYAN, 120, "thatch-up");
        #endif
        sprintf(sFilename, "th-%s-up.bmp", sGetBase(*psImage));
        lprintf("%s ", sFilename);
        WriteBmp(Img, sFilename);
#endif
        }
    lprintf("\n");
    psImage++;
    }
return 0;
}
