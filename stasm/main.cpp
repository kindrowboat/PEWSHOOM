// $main.cpp 3.0 milbo$ main routine for locating face features using stasm code
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

static const char *LOG_FILE_NAME = "stasm.log";

// CONVERT_TO_OPENCV_COORDS applies to the coords printed in the log file.
// Set it to true to convert our internal shape coords (with 0,0 at
// center of image) to standard image coords (with 0,0 at top left of image)

static const bool CONVERT_TO_OPENCV_COORDS = false;

static const bool SHOW_ME17_POINTS = false;

static const char *sgUsage =
"Usage: stasm [FLAGS] IMAGE [IMAGE2 ...]\n"
"\n"
"Flags:\n"
"\n"
"-r use Rowley face detector (default is Viola Jones)\n"
"\n"
"-i[sSfF]\n"
"    Write images (default is -iF meaning write image showing final shape)\n"
"    -iS    start shape\n"
"    -if    no final shape\n"
"\n"
"-c FILE.conf\n"
"    Config file name\n"
"    Use -c twice to specify two stacked models\n"
"    Default ../data/mu-68-1d.conf\n"
"        and ../data/mu-76-2d.conf\n"
"\n"
"-p PATTERN\n"
"    Use IMAGE only if its name matches case-independent regex PATTERN\n"
"\n"
"-P Mask0 Mask1  (can only be used with the -t flag)\n"
"    Load only shapes which satisfy (Attr & Mask0) == Mask1\n"
"    Mask1=ffffffff is treated specially, meaning satisfy (Attr & Mask0) != 0\n"
"\n"
"-t TABFILE SHAPEFILE N\n"
"    Read images and face dets params from SHAPEFILE, write fits to TABFILE\n"
"    If N is not zero, select a sample of N shapes from shape file (after applying -p and -P)\n"
"    Example: -t test.tab ../data/muct68.shape 100\n"
"\n"
"This is stasm %s\n";           // %s is STASM_VERSION

//-----------------------------------------------------------------------------
// Get the fit of Shape agains the shape with name sBase in sShapeFile.
// If CONF_fMe17, then only the me17 points are used.
// If CONF_fXM2vts, the distances are scaled by the intereye distance.

static void GetFit (double &Fit,               // out: overall fitness
                    Vec &Fits,                 // out:
                    double &InterEyeDist,      // out: intereye distance in ref shape
                    double &FaceWidth,         // out: face width in ref shape
                    const SHAPE &Shape,        // in:
                    const char sBase[],        // in: base name of image
                    const char sShapeFile[],   // in: look for sBase in this file
                    const RgbImage *pImg=NULL) // in: for debugging
{
// Change xxx to ^xxx$ to avoid spurious matches else B00 would match B0012
// The FA_ViolaJones|FA_Rowley means ignore face detector shapes

char s[SLEN]; sprintf(s, "^%s$", sBase);
SHAPE RefShape = FindMatInFile(sShapeFile, NULL, s, FA_ViolaJones|FA_Rowley, 0);

if (RefShape.nrows() == 0)
    Err("GetFit: %s is not in %s", sBase, sShapeFile);  // was Warn in stasm 2.4

// found RefShape in the file

SHAPE Shape17(Shape);
SHAPE Ref17(RefShape);
InterEyeDist = 1;
FaceWidth = xShapeExtent(RefShape);
if (CONF_fMe17)
    {
    ConvertToShape17(Ref17, RefShape);
    ConvertToShape17(Shape17, Shape);
    InterEyeDist = PointToPointDist(Ref17, 0, Ref17, 1);
    }
else if (CONF_fXm2vts)
    InterEyeDist = PointToPointDist(RefShape, MLEye, RefShape, MREye);
const unsigned nPoints = Shape17.nrows();
if (nPoints != Ref17.nrows())
    Err("%s in %s has a number of points %d different "
        "from the points %d in the ASM model%s",
        sBase, sShapeFile, Ref17.nrows(), nPoints,
        CONF_fMe17? " (after conversion to 17 points)": "");
Fit = 0;
Fits.dim(nPoints, 1);
for (unsigned iPoint = 0; iPoint < nPoints; iPoint++)
    {
    if (!fPointUsed(Shape17, iPoint) && fPointUsed(Ref17, iPoint))
        Err("GetFit %s point %d unused %s", sBase, iPoint,
            CONF_fMe17? " (after conversion to 17 points)": "");
    Fits(iPoint) = PointToPointDist(Shape17, iPoint, Ref17, iPoint);
    Fit += Fits(iPoint);
    }
Fit /= nPoints;
Fit /= InterEyeDist;
Fits /= InterEyeDist;

if (SHOW_ME17_POINTS && pImg)
    {
    RgbImage TempImg(*pImg);
    DrawShape(TempImg, Ref17,   C_YELLOW, true, C_YELLOW, true);
    DrawShape(TempImg, Shape17, C_RED,    true, -1, true);
    char sPath[SLEN];
    sprintf(sPath, "me17-%s.bmp", sBase);
    lprintf("Writing %s ", sPath);
    WriteBmp(TempImg, sPath);
    }
}

//-----------------------------------------------------------------------------
// write Img, with StartShape superimposed
// image file name is prefixed with "1"

static void ShowStartShape (const SHAPE &StartShape,        // in: all
                            const DET_PARAMS &DetParams,
                            const Image &Img,
                            const char sBase[],             // basename of image
                            const bool fRowley)
{
RgbImage TempImg(Img);  // local copy that we can write into
SHAPE DetShape = DetParamsToShape(DetParams);
DrawShape(TempImg, DetShape, C_YELLOW, true, C_YELLOW);
DrawShape(TempImg, StartShape, C_RED);
CropImageToShape(TempImg, DetShape);

char sPath[SLEN];
sprintf(sPath, "1%s-%s.bmp", (fRowley? "-row": ""), sBase);
lprintf("Writing %s ", sPath);

WriteBmp(TempImg, sPath);
}

//-----------------------------------------------------------------------------
// write Img, with Shape superimposed

void ShowFinalShape (const SHAPE &Shape,    // in: all
                     const RgbImage &Img,
                     const char sBase[],    // basename of image
                     const bool fRowley)
{
RgbImage TempImg(Img);  // local copy that we can write into
DrawShape(TempImg, Shape, C_RED);
CropImageToShape(TempImg, Shape);

char s[SLEN];
sprintf(s, "s%s-%s.bmp", (fRowley? "-row": ""), sBase);
lprintf("Writing %s ", s);

WriteBmp(TempImg, s);
}

//-----------------------------------------------------------------------------
static void ShowResults (const SHAPE Shape,         // in: the final shape
                         const SHAPE StartShape,
                         const DET_PARAMS &DetParams,
                         const char sImage[],
                         const RgbImage &Img,
                         const char sTabFile[],
                         const char sShapeFile[],
                         double MeanTime,
                         const bool fRowley,
                         const bool fShowStartShape,
                         const bool fShowFinalShape,
                         FILE *pLogFile)             // in: where to write results
{
char *sBase = sGetBase(sImage); // base name of image

if (sShapeFile && sTabFile)
    {
    double StartFit, FinalFit;
    Vec Fits; // size will be allocated in GetFit
    double InterEyeDist = 0, FaceWidth = 0;
    GetFit(StartFit, Fits, InterEyeDist, FaceWidth, StartShape, sBase, sShapeFile);
    GetFit(FinalFit, Fits, InterEyeDist, FaceWidth, Shape, sBase, sShapeFile, &Img);
    AppendToTabFile(sTabFile, sBase, StartFit, FinalFit, Fits,
                    InterEyeDist, FaceWidth);
    printf("MeanTime %-4.3f Fit %-8.3f ", MeanTime, FinalFit);
    }
if (fShowStartShape)
    ShowStartShape(StartShape, DetParams, Img, sBase, fRowley);

if (fShowFinalShape)
    ShowFinalShape(Shape, Img, sBase, fRowley);

// print coordinates of Shape to the log file.

SHAPE TempShape(Shape);
if (CONVERT_TO_OPENCV_COORDS)
    {
    TempShape.col(VX) += Img.width / 2;
    TempShape.col(VY) = Img.height / 2 - TempShape.col(VY);
    }
logprintf("\n");
TempShape.write(LOG_FILE_NAME, pLogFile, "%8.2f", sBase);
}

//-----------------------------------------------------------------------------
// Locate landmarks in given image(s), write results to pLogFile
// and also possibly write images, depending on flags passed in

static void LocateLandmarks (
    const char *sImageNames[],   // in: image filenames, if NULL use Tags instead
    int nImages,                 // in: number of images
    bool fRowley,                // in: true to use Rowley, else Viola Jones
    const char sConfFile0[],     // in: 1st config filename
    const char sConfFile1[],     // in: 2nd config filename, "" if none
    const char sDataDir[],       // in: data directory
    const char sTabFile[],       // in: if not NULL generate sTabFile
    const char sShapeFile[],     // in: if not NULL lookup det results in sShapeFile
    const bool fShowStartShape,  // in:
    const bool fShowFinalShape,  // in:
    FILE *pLogFile,              // in: where to write the results
    char sTagRegex[],            // in: -p command line flag
    vec_string Tags,             // in: tags from shape file, only if -t command line flag
    char sImageDirs[])
{
double MeanTime;
int nFound = 0, nUsed = 0;
char sPath[SLEN];

// regcomp flags: egrep style expression (supports |),
// ignore case, use simple failure reporting in regexec

regex_t CompiledRegex;
if (sTagRegex[0] && 0 != regcomp(&CompiledRegex, sTagRegex,
                              REG_EXTENDED|REG_ICASE|REG_NOSUB))
    {
    Err("invalid -p regular expression %s", sTagRegex);
    }
for (int iImage = 0; iImage < nImages; iImage++)
    {
    bool fUse = true;
    const char *sImage;
    if (sImageNames)
        {
        sImage = sImageNames[iImage];
        if (sTagRegex[0])
            {
            // only use the file if its basename matches sTagRegex
            char sBase[_MAX_FNAME];
            splitpath(sImage, NULL, NULL, sBase, NULL);
            regmatch_t pmatch[1];
            if (regexec(&CompiledRegex, sBase, 0, pmatch, 0))
                fUse = false;
            }
        }
    else
        {
        sGetPathGivenDirs(sPath, sGetBasenameFromTag(Tags[iImage].c_str()), sImageDirs, sShapeFile);
        sImage = sPath;
        }
    if (fUse)
        {
        // read the image from disk

        nUsed++;
        if (nImages > 3)
            lprintf("%d Reading %s ", nUsed, sImage);
        else
            lprintf("Reading %s ", sImage);
        RgbImage Img(sImage);

        // locate the face landmarks, put them into Shape

        SHAPE StartShape;
        DET_PARAMS DetParams;   // face detector parameters
        SHAPE Shape = AsmSearch(StartShape, DetParams, MeanTime,
                                Img, sImage, fRowley,
                                sConfFile0, sConfFile1, sDataDir, sShapeFile);

        if (Shape.nrows())      // successful location of landmarks?
            {
            nFound++;
            ShowResults(Shape, StartShape, DetParams, sImage, Img,
                        sTabFile, sShapeFile, MeanTime, fRowley, fShowStartShape,
                        fShowFinalShape, pLogFile);
            }
        else if (fShowFinalShape)   // landmark location failed, show image anyway
            {
            char s[SLEN];
            sprintf(s, "s-%s-noface.bmp", sGetBase(sImage));
            lprintf("Writing %s ", s);
            WriteBmp(Img, s);
            }
        lprintf("\n");
        }
    }
if (sTabFile)
    CloseTabFile(nImages, nFound, MeanTime);
}

//-----------------------------------------------------------------------------
int
main (int argc, const char *argv[])
{
int argc1 = argc;               // save for PrintCmdLine()
const char **argv1 = argv;

const char *sUseQForHelp = "Use stasm -? for help.";

char sConfFile0[SLEN] = "../data/mu-68-1d.conf";
char sConfFile1[SLEN] = "../data/mu-76-2d.conf";

bool fRowley = false;           // -r command line flag
const char *sTabFile = NULL;    // -t command line flag, 1st arg
const char *sShapeFile = NULL;  // -t command line flag, 2nd arg
int nMaxShapes = 0;             // -t command line flag, 3rd arg
bool fShowStartShape = false;   // -iS: write bmp showing start shape
bool fShowFinalShape = true;    // -iF: write bmp showing final shape
static char sTagRegex[10000]; sTagRegex[0] = 0; // -p command line flag, static because so big
unsigned Mask0 = 0, Mask1 = 0;  // -P command line flag

int nModels = 0;
char sDataDir[SLEN];            // data dir (for face detector files etc.)
GetDataDir(sDataDir, *argv);

while (--argc > 0 && (*++argv)[0] == '-')
    {
    const char c = *(*argv + 1);

    if (c != 'i' && *(*argv + 2))
        Err("extra character '%c' after -%c.  %s",
            *(*argv + 2), c, sUseQForHelp);

    switch (c)
        {
        case 'c':               // -c FILE.conf
            argv++;
            argc--;
            if (argc < 1 || *(*argv) == '-')
                Err("-c needs an argument.  %s", sUseQForHelp);
            if (nModels == 0)
                {
                strcpy(sConfFile0, *argv);
                sConfFile1[0] = 0;
                }
            else if (nModels == 1)
                strcpy(sConfFile1, *argv);
            else
                Err("too many -c flags (up to two are allowed).  %s",
                    sUseQForHelp);
            nModels++;
            break;
        case 'r':               // -r
            fRowley = true;
            break;
        case 'p':               // -p PATTERN
            argv++;
            argc--;
            if (argc < 1 || *(*argv) == '-')
                Err("-p needs an argument.  %s", sUseQForHelp);
            if (*argv[0] == '-')
                Err("Illegal string for -p flag.  Use -? for help.");
            strcpy(sTagRegex, *argv);
            break;
        case 'P':              // -p Mask0 Mask1
            if (!sShapeFile)
                Err("-P can only be used with the -t flag");
            argv++;
            argc--;
            if (argc < 1 || *(*argv) == '-'  || 1 != sscanf(*argv, "%x", &Mask0))
                Err("-P needs two hex numbers.  Use -? for help.");
            argv++;
            argc--;
            if (argc < 1 || *(*argv) == '-' || 1 != sscanf(*argv, "%x", &Mask1))
               Err("-P needs two hex numbers.  Use -? for help.");
        break;
        case 't':               // -t TAB.tab SHAPE.shape N SEED
            {
            argv++;
            argc--;
            if (argc < 1 || *(*argv) == '-')
                Err("-t needs three arguments.  %s", sUseQForHelp);
            sTabFile = *argv;
            int n = strlen(sTabFile);
            // prevent the user from overwriting a shape file by mistake
            if (n < 4 || stricmp(sTabFile + n - 4, ".tab"))
                {
                if (n >= 6 &&
                        0 == stricmp(sTabFile + n - 6, ".shape") &&
                        *(*(argv+1)) != '-')
                    Err("the .shape file must be the SECOND argument of -t."
                        "  %s", sUseQForHelp);
                else
                    Err("the first argument of -t must be TABFILE.tab "
                        "(with a .tab suffix).  %s", sUseQForHelp);
                }
            argv++;
            argc--;
            if (argc < 2 || *(*argv) == '-')
                Err("-t needs three arguments.  %s", sUseQForHelp);
            sShapeFile = *argv;
            n = strlen(sShapeFile);
            if (n < 6 || stricmp(sShapeFile + n - 6, ".shape"))
                Err("the second argument of -t must be SHAPEFILE.shape "
                    "(with a .shape suffix).  %s", sUseQForHelp);
            argv++;
            argc--;
            if (argc < 1 || *(*argv) == '-')
                Err("-t needs three arguments.  %s", sUseQForHelp);
            if (1 != sscanf(*argv, "%d", &nMaxShapes) || nMaxShapes < 0 || nMaxShapes > 1e6)
               Err("illegal N argument for -t flag (expected a number).  Use -? for help.");
#if 0 // extra CONF_nSeed_SelectShapes parameter
            argv++;
            argc--;
            // Note CONF_nSeed_SelectShapes can't be in the conf file
            // because we need it before reading the conf file
            if (argc < 1 || *(*argv) == '-')
                Err("-t needs four arguments.  %s", sUseQForHelp);
            if (1 != sscanf(*argv, "%d", &CONF_nSeed_SelectShapes) ||
                    CONF_nSeed_SelectShapes < 0 || CONF_nSeed_SelectShapes > 10000)
               Err("illegal SEED for -t flag (expected a nonnegative number).  Use -? for help.");
#endif
            break;
            }
        case 'i':               // -is -if -iS -iF
            {
            const char c1 = *(*argv + 2);
            switch (c1)
                {
                case 's': fShowStartShape = false; break;
                case 'S': fShowStartShape = true;  break;
                case 'f': fShowFinalShape = false; break;
                case 'F': fShowFinalShape = true;  break;
                default:
                    Err("illegal flag -%s.  %s", *argv + 1, sUseQForHelp);
                }
            }
            break;
        case '?':               // -?
            lprintf(sgUsage, STASM_VERSION);
            exit(1);
        default:
            Err("illegal flag -%s.  %s", *argv + 1, sUseQForHelp);
        }
    }
if (sShapeFile)
    {
    if (argc != 0)
        Err("you used the -t flag and thus cannot specify images on the command line,\n"
            "because the image names are taken from the shape file.\n"
            "%s", sUseQForHelp);
    }
else if (argc < 1)
    Err("no image.  %s", sUseQForHelp);

// after pOpenLogFile(), lprintfs will go to the screen and to the log file
FILE *pLogFile = pOpenLogFile(LOG_FILE_NAME, VERBOSE);
logprintf("stasm %s\n", STASM_VERSION);
PrintCmdLine(pLogFile, argc1, argv1);

int nImages = argc;
static vec_string Tags;       // array[nShapes] of string preceding each shape
char sImageDirs[SLEN];
if (sShapeFile)
    {
    static vec_SHAPE Shapes; // the matrices read in from the file
    argv = NULL;
    ReadSelectedShapes(Shapes, Tags, sImageDirs, sShapeFile,
                       nMaxShapes, sTagRegex, Mask0, Mask1);
    Shapes.clear();         // don't actually need the shapes, just the tags
    nImages = Tags.size();
    if (nImages == 0)
        Err("no tags in %s match -P \"%s\" -p %x %x", sShapeFile, sTagRegex, Mask0, Mask1);
    lprintf("%d image%s\n", nImages, (nImages == 1? "": "s"));
    }
else
    logprintf("%d image%s\n", nImages, (nImages == 1? "": "s"));

LocateLandmarks(argv, nImages, fRowley,
                sConfFile0, sConfFile1, sDataDir, sTabFile, sShapeFile,
                fShowStartShape, fShowFinalShape, pLogFile, sTagRegex,
                Tags, sImageDirs);

ShutdownStasm();
return 0;   // 0 means success
}
