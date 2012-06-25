// $fdet.cpp 3.0 milbo$ console front end to VJ and Rowley face detectors
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

static const char *FDET_VERSION = "version 3.0";

static const char *sgUsage =
"Usage: fdet [-r] [-i] [-n] ImageFile [ImageFile2 ...]\n"
"\n"
"-r use Rowley detector (default is Viola Jones)\n"
"\n"
"-i write image showing detector shape\n"
"\n"
"-n no crop image to face (only applies if -i)\n"
"\n"
"This is fdet %s\n";   // %s is FDET_VERSION

//-----------------------------------------------------------------------------
// write the input image with the detector shape superimposed

static void WriteResultsImage (const char sFile[],
                               const DET_PARAMS &DetParams,
                               bool fSuccess,
                               bool fRowley,
                               bool fCrop)
{
const char *sBase = sGetBase(sFile);
RgbImage Img;
sLoadImage(Img, sFile, QUIET);
if (fSuccess)
    {
    SHAPE DetShape = DetParamsToShape(DetParams);

    DrawShape(Img, DetShape, C_YELLOW, true, C_YELLOW);
    if (Img.width > 500)
        {
        // draw again because can't easily see 1 pixel width lines on big images
        DrawShape(Img, DetShape+1, C_YELLOW, true, C_YELLOW);
        }
    if (Img.width > 1000)
        DrawShape(Img, DetShape-1, C_YELLOW, true, C_YELLOW);

    if (fCrop)
        CropImageToShape(Img, DetShape);
    }
#if _MSC_VER // RgbPrintf only supported under MSC
else
    RgbPrintf(Img, Img.width - 80, 0, C_YELLOW, 80, "no face");
#endif

char sPath[SLEN];
sprintf(sPath, "out/%s-%s.bmp", (fRowley? "row-": "vj-"), sBase);
printf("Writing %s ", sPath);
WriteBmp(Img, sPath);
}

//-----------------------------------------------------------------------------
int main (int argc, const char *argv[])
{
int argc1 = argc;               // save for PrintCmdLine()
const char **argv1 = argv;

bool fWriteImage = false;       // -i command line flag
bool fRowley = false;           // -r command line flag
bool fCrop = true;              // -c command line flag

const char *sUseQForHelp = "Use fdet -? for help.";
while (--argc > 0 && (*++argv)[0] == '-')
    {
    const char c = *(*argv + 1);
    if (*(*argv + 2))
        Err("extra character '%c' after -%c.  %s",
            *(*argv + 2), c, sUseQForHelp);
    switch (c)
        {
        case 'i':               // -i
            fWriteImage = true;
            break;
        case 'r':               // -r
            fRowley = true;
            break;
        case 'n':               // -n (only applies if -i)
            fCrop = false;
            break;
        case '?':               // -?
            printf(sgUsage, FDET_VERSION);
            exit(1);
        default:
            Err("illegal flag -%s.  %s", *argv + 1, sUseQForHelp);
        }
    }
if (argc < 1)
    Err("no image.  %s", sUseQForHelp);

pgLogFile = pOpenLogFile("fdet.log", QUIET);
printf("Writing fdet.log for %d file%s\n",
    argc, (argc == 1? "": "s"));
PrintCmdLine(stdout, argc1, argv1);

// hack to make fViolaJonesFindFaceAndEyes() process eyes too
CONF_nVjMethod = 0;

unsigned DetAttr = (fRowley? FA_Rowley: FA_ViolaJones);
int nFiles = 0, nFaces = 0, nNoEyes = 0, nOneEye = 0;
const clock_t BeginTime = clock();
while (--argc >= 0)
    {
    const clock_t StartTime = clock();
    const char *sFile = *argv++;
    printf("[%d] Processing %s ", ++nFiles, sFile);
    fflush(stdout);

    DET_PARAMS DetParams;
    bool fSuccess = fFindDetParams(DetParams,
                                   sFile, DetAttr,  "../data", false);

    // PossiblyIssueDetWarning(fSuccess, DetAttr, DetParams);

    if (fWriteImage)
        WriteResultsImage(sFile, DetParams, fSuccess, fRowley, fCrop);

    printf("[%.3f secs]\n", double(clock() - StartTime) / CLOCKS_PER_SEC);
    fflush(stdout);
    if (fSuccess)
        {
        nFaces++;
        if (DetParams.lex == INVALID && DetParams.rex == INVALID)
            nNoEyes++;
        else if (DetParams.lex == INVALID || DetParams.rex == INVALID)
            nOneEye++;
        if (DetParams.lex == INVALID)
            DetParams.ley = INVALID;
        if (DetParams.rex == INVALID)
            DetParams.rey = INVALID;

        // log the face detector parameters (so we can extract them
        // manually later and put them into the shape file)

        logprintf("\"%4.4x %s\"\n"
                  "{ 1 8 %.0f %.0f %.0f %.0f %.0f %.0f %.0f %.0f\n}\n",
                  DetAttr, sGetBase(sFile),
                  DetParams.x, DetParams.y,
                  DetParams.width, DetParams.height,
                  DetParams.lex, DetParams.ley,
                  DetParams.rex, DetParams.rey);
        }
    else
        logprintf("# %s: no face\n",
                  sGetBase(sFile), sGetDetString(DetAttr));
    }
lprintf("Found %d faces in %d files, %.1f%% no eyes %.1f%% one eye "
       "[%.3f secs per file]\n",
        nFaces, nFiles,
        100 * double(nNoEyes)/nFiles,
        100 * double(nOneEye)/nFiles,
        double(clock() - BeginTime) / (CLOCKS_PER_SEC * nFiles));

ShutdownStasm();
return 0;
}
