// $tasm.cpp 3.0 milbo$ command line utility to build an active shape model
// Cape Town jun 08
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
#include "tasm.hpp"
#include "tasmshapes.hpp"
#include "detav.hpp"

static const char TASM_VERSION[] = "version 3.0";

// Information on all shapes, initialized from shape file by ReadTrainingShapes.
// In the comments below nShapes is Shapes.size()
//                       nPoints is Shapes[iRefShape].nrows()

typedef struct SH
    {
    int         iRefShape;  // index of shape used to align other shapes

    char        sImageDirs[SLEN];  // image directories at head of shape
                                   // file, separated by semicolons

    SHAPE       AvShape;    // average shape after aligning shapes
    SHAPE       VjAv;       // mean shape relative to Viola Jones frame
    SHAPE       RowleyAv;   // mean shape relative to Rowley frame

    vec_SHAPE    OrgShapes; // vec [nShapes] of SHAPE: original shapes from file
                            // but with extra eye points if CONF_fSynthEyePoints

    vec_SHAPE    Shapes;    // vec [nShapes] of SHAPE: OrgShapes, but aligned

    vec_string   Tags;      // vec [nShapes] of string:
                            // string tag preceding each shape in shape file

    vec_int      TagInts;   // vec [nShapes] of unsigned:
                            // hex number at start of each of above tags

    vec_int      nUsedLandmarks; // vec [nShapes] of int, used for skipping
                                 // unused landmarks i.e. at posn 0,0

                                      // following vars for CONF_fSynthEyePoints
    vec_bool     fExtraLEyeLandsUsed; // vec [nShapes] of bool
    vec_bool     fExtraREyeLandsUsed; // vec [nShapes] of bool
    }
SH;

typedef vector<vec_Mat> vec_vec_Mat; // a vec of mats for each point
                                     // for each level in the image pyr

typedef struct STATS   // everything you ever wanted to know about the profiles for
    {                  // all shapes and images at all pyramid levels

    vector<vec_int> nProfs;  // vec (nLevs][nPoints]: nbr of profiles for each point

    vec_vec_Mat     Covars;  // vec [nLevs][nPoints] of covar mats:
                             // each is nProfWidth x nProfWidth

    vec_vec_Mat     AvProfs; // vec [nLevs][nPoints] of profile vecs averaged
                             // over all images: each is 1 x nProfWidth

    vec_vec_Mat     Profs;   // vec [nLevs][nPoints] of profile mats:
                             // each is nShapes x nProfWidth
    }
STATS;

static const char sgUsage[] =
    "Usage: tasm [-o OUTFILE.asm] FILE.conf\n"
    "\n"
    "Creates ASM file OUTFILE.asm from config file FILE.conf\n"
    "where FILE.conf is a config file\n"
    "\n"
    "-o OUTFILE.asm\n"
    "    Output file [default is temp.asm]\n";

//-----------------------------------------------------------------------------
// Configuration parameters.  The defaults below may be modified when
// GetConfParams reads in the .conf file.

static char CONF_sShapeFile[SLEN];  // input shapefile, no default

static int CONF_nMaxShapes = 0;     // max number of shapes (0 for all)

// About CONF_sTagRegex:
// We load only shapes with tags matching sTagRegex
// which is an egrep-style case-independent regular expression.
// Example: sTagRegex="xyz"        loads filenames containing xyz.
// Example: sTagRegex="m000| m001" loads filenames beginning with m000 or m001.
// Default: sTagRegex=""           loads all shapes.

static char CONF_sTagRegex[SLEN];

// About CONF_AttrMask0 and CONF_AttrMask1:
// We load only shapes whose Attr match Mask0 and Mask1 (see fMatchAttr).
// Attr is the hex number at start of the tag, Mask0 and Mask1 are hex.
// In older versions of masm these were the -P command line flag.
// This filter is applied after sTagRegex.
// Example: Mask0=2 Mask1=2 matches faces with glasses (FA_Glasses=2, see atface.hpp).
// Example: Mask0=2 Mask1=0 matches faces without glasses.
// Default: Mask0=0 Mask1=0 i.e. no filter,  match all shapes.

static unsigned CONF_AttrMask0 = 0;
static unsigned CONF_AttrMask1 = 0;

// Generate 2d profs for pyr levels less or equal to this,
// so -1 means no 2d profiles i.e. all 1d profiles

int CONF_nLev2d = -1;

// Select which points have 2D profiles, only used if CONF_nLev2d >= 0
// See GEN2D_All etc.c defs in tasm.hpp
// You can extend this by modifying GetGenProfSpec, see that func for details.

eGen2d CONF_nWhich2d = GEN2D_All;

// profile type for the 1D profiles, see defs in prof.hpp

unsigned CONF_ProfType = PROF_Grad|PROF_Flat; // classic Cootes 1-dimensional profile

// profile type for the 2D profiles

unsigned CONF_ProfType2d = (PROF_WindowEquallyWeighted|PROF_SigmAbsSum|PROF_GradBelowRight);

static int CONF_nLevs = 4;      // generate this many levels in image pyramid
                                // 4 works well for a pyramid ratio of 2

static double CONF_PyrRatio = 2.0; // size of this image pyramid
                                   // level versus the next pyramid level

static int CONF_PyrReduceMethod = IM_NEAREST_PIXEL;
                        // one of IM_NEAREST_PIXEL=0 IM_BILINEAR=1 IM_AVERAGE_ALL=2

int CONF_nProfWidth1d = 8*2+1;  // nbr of 1D profile points: both sides and center
int CONF_nProfWidth2d = 6*2+1;  // ditto but in 2 dimensions for 2D profiles
static int CONF_nTrimCovar = 3; // trim the covariance matrices, 0 for no trim

double CONF_MaxShapeAlignDist = 1e-6; // for convergence test in AlignShapes

static int CONF_nStandardFaceWidth = 180;   // use 0 for no face prescaling
static bool CONF_fPrescaleBilinear = true;  // prescale using nearest-pix or bilin

static double CONF_SigmoidScale = 1000; // curviness of sigmoid, 0 for no sigmoid
                                        // only used for 2D profiles
int CONF_nSeed_ShapeNoise = 1234;

double CONF_ShapeNoise = 0;             // random x and y noise addded to
                                        // landmark positions (0 for no noise)

double CONF_xRhsStretchShape = 0;       // random x stretch of right hand
                                        // side of shapes (0 for no distortion)

double CONF_xStretchShape = 0;          // random x stretch of
                                        // shapes (0 for no stretching)

bool CONF_fXm2vts = false; // 1 to use (extended) XM2VTS tables in landmarks.hpp

// Synthesize missing eye points from other eye

static bool CONF_fSynthEyePoints = false;

static bool CONF_fTasmSkipIfNotInShapeFile = false;

// 0=use classic Cootes method i.e. ordering of points in shape
// 1=use Lands.iPrev and Lands.iNext
// Can only be used if fXm2vts is true (because needs gLandTab)

bool CONF_fExplicitPrevNext = false;

// True to skip points behind glases etc.  See fPointObscured().
// Can only be used if fXm2vts (because needs gLandTab).

static bool CONF_fUnobscuredFeats = false;

// Table of parameters that can be changed in the .conf file
// Min and Max limits in the table below are more-or-less arbitrary
//
// Note: when changing this table, be very careful that the Type char
// matches the actual type of p.  Else fReadEntry will write into
// the wrong place in the stack, which is a hard problem to debug.

static CONF_ENTRY TasmConfTab[] =
{
// p                         sName                   Min Max    Type
{&CONF_sShapeFile,           "sShapeFile",             0, 0,      's'},
{&CONF_nMaxShapes,           "nMaxShapes",             0,100000,  'i'},
{&CONF_fSynthEyePoints,      "fSynthEyePoints",        0, 1,      'f'},
{&CONF_fTasmSkipIfNotInShapeFile,"fTasmSkipIfNotInShapeFile", 0,1,'f'},
{&CONF_sTagRegex,            "sTagRegex",              0, 0,      's'},
{&CONF_AttrMask1,            "AttrMask1",              0, 0xffff, 'x'},
{&CONF_AttrMask0,            "AttrMask0",              0, 0xffff, 'x'},
{&CONF_nLevs,                "nLevs",                  1, 10,     'i'},
{&CONF_PyrRatio,             "PyrRatio",               1,  4,     'd'},
{&CONF_nLev2d,               "nLev2d",                -1, 10,     'i'},
{&CONF_nWhich2d,             "nWhich2d", GEN2D_All,GEN2D_InternalExceptNose,'i'},
{&CONF_ProfType,             "ProfType",               0, 0xffff, 'x'},
{&CONF_ProfType2d,           "ProfType2d",             0, 0xffff, 'x'},
{&CONF_PyrRatio,             "PyrRatio",               1, 10,     'd'},
{&CONF_PyrReduceMethod,      "PyrReduceMethod",        0, 2,      'i'},
{&CONF_nProfWidth1d,         "nProfWidth1d",           1, 100,    'i'},
{&CONF_nProfWidth2d,         "nProfWidth2d",           1, 100,    'i'},
{&CONF_nTrimCovar,           "nTrimCovar",             0, 100,    'i'},
{&CONF_MaxShapeAlignDist,    "MaxShapeAlignDist",      0, 1e-2,   'd'},
{&CONF_nStandardFaceWidth,   "nStandardFaceWidth",     1, 1000,   'i'},
{&CONF_fPrescaleBilinear,    "fPrescaleBilinear",      0, 1,      'f'},
{&CONF_fXm2vts,              "fXm2vts",                0, 1,      'f'},
{&CONF_fExplicitPrevNext,    "fExplicitPrevNext",      0, 1,      'f'},
{&CONF_fUnobscuredFeats,     "fUnobscuredFeats",       0, 1,      'f'},
{&CONF_NormalizedProfLen,    "NormalizedProfLen",      1, 1000,   'd'},
{&CONF_SigmoidScale,         "SigmoidScale",           0, 1e5,    'd'},
{&CONF_nSeed_ShapeNoise,     "nSeed_ShapeNoise",      0,100000,   'i'},
{&CONF_nSeed_SelectShapes,   "nSeed_SelectShapes",    0,100000,   'i'},
{&CONF_ShapeNoise,           "ShapeNoise",             0, 10,     'd'},
{&CONF_xRhsStretchShape,     "xRhsStretchShape",       0, 1,      'd'},
{&CONF_xStretchShape,        "xStretchShape",          0, 1,      'd'},
{&CONF_nSleep,               "nSleep",                 0, 500,    'i'}
};

//-----------------------------------------------------------------------------
// Check that each subprofile has only one subprofile
// (ms and masm support multiple subprofiles but not tasm and stasm).

static void Check_nSubProfsIsOne (int nPoints)
{
for (int iLev = 0; iLev < CONF_nLevs; iLev++)
    for (int iPoint = 0; iPoint < nPoints; iPoint++)
        {
        unsigned ProfSpec = GetGenProfSpec(iLev, iPoint);
        if (ProfSpec & 0xffff00)
            Err("iLev %d iPoint %d has multiple subprofiles\n"
                "       tasm allows only one subprofile per profile -- "
                "check GenGenProfSpec", iLev, iPoint);
        }
}

//-----------------------------------------------------------------------------
// Allocate space in Stats
// This also inits sProfSpecs which is a string listing all the
// profile types (for user information only).

static void AllocStats (STATS &Stats,      // io
                        char sProfSpecs[], // out: only for printing info to console
                        const SH &Sh)      // in
{
ASSERT(IS_ODD(CONF_nProfWidth1d));
ASSERT(IS_ODD(CONF_nProfWidth2d));

const int nPoints = Sh.Shapes[Sh.iRefShape].nrows();
// max nbr of profs per landmark, not all used
// if CONF_fUnobscuredFeats is true
const int nMaxProfsPerPoint = Sh.Shapes.size();
Stats.nProfs.resize(CONF_nLevs);
Stats.Covars.resize(CONF_nLevs);
Stats.Profs.resize(CONF_nLevs);
Stats.AvProfs.resize(CONF_nLevs);

static unsigned LastProfSpec = 0;
sProfSpecs[0] = 0;
for (int iLev = 0; iLev < CONF_nLevs; iLev++)
    {
    Stats.nProfs[iLev].resize(nPoints, 0);
    Stats.Covars[iLev].resize(nPoints, 0);
    Stats.Profs[iLev].resize(nPoints, 0);
    Stats.AvProfs[iLev].resize(nPoints, 0);

    for (int iPoint = 0; iPoint < nPoints; iPoint++)
        {
        ASSERT(fPointUsed(Sh.AvShape, iPoint));
        unsigned ProfSpec = GetGenProfSpec(iLev, iPoint);

        int nProfPoints = nGenElemsPerSubProf(ProfSpec);

        Stats.Covars[iLev][iPoint].dimClear(nProfPoints, nProfPoints);

        Stats.Profs[iLev][iPoint].dimClear(nMaxProfsPerPoint, nProfPoints);

        Stats.AvProfs[iLev][iPoint].dimClear(1, nProfPoints);

        // if ProfSpec changed, append it to the sProfSpecs string

        if (LastProfSpec != ProfSpec)
            {
            char s[SLEN]; sprintf(s, "%d,%d:%8.8x ", iLev, iPoint, ProfSpec);
            strcat(sProfSpecs, s);
            LastProfSpec = ProfSpec;
            }
        }
    }
}

//-----------------------------------------------------------------------------
// Return true if a point is obscured by glasses, beard, or mustache.
// This can only be for XM2VTS or extended XM2VTS points because
// we use gLandTab to determine if a point is obscured.

bool fPointObscured (const vec_int &TagInts, int iShape, int iPoint)
{
ASSERT(!CONF_fUnobscuredFeats || CONF_fXm2vts); // needs gLandTab

return CONF_fUnobscuredFeats &&
        (0 !=
            (TagInts[iShape] &
            gLandTab[iPoint].Bits &
            (FA_Glasses|FA_Beard|FA_Mustache)));
}

//-----------------------------------------------------------------------------
static bool fMustCollectProf (const SH &Sh,             // in
                              int iShape,               // in
                              int iPoint,               // in
                              const SHAPE ScaledShape)  // in
{
bool fCollect = true;

if (!fPointUsed(ScaledShape, iPoint))
    fCollect = false;
else if (fPointObscured(Sh.TagInts, iShape, iPoint))
    fCollect = false;
else if (CONF_fSynthEyePoints)
    {
    if (iPoint >= MLEye0 && iPoint <= MLEye7) // can we collect left eye prof?
        {
        if (!Sh.fExtraLEyeLandsUsed[iShape])  // not if left eye point is unused
            fCollect = false;
        }
    else if (iPoint >= MREye0 && iPoint <= MREye7) // can we collect right eye prof?
        {
        if (!Sh.fExtraREyeLandsUsed[iShape]) // not if right eye point is unused
            fCollect = false;
        }
    }
return fCollect;
}

//-----------------------------------------------------------------------------
// Get the factor needed to scale the face width to CONF_nStandardFaceWidth
// But if CONF_nStandardFaceWidth==0 then return 1

static double GetFaceScale (const SHAPE &Shape, int nStandardFaceWidth)
{
double FaceScale = 1;

if (nStandardFaceWidth != 0)
    FaceScale = nStandardFaceWidth / xShapeExtent(Shape);

return FaceScale;
}

//-----------------------------------------------------------------------------
// Collect the profile for one landmark.
// Put the profile into row Stats.ProfspiLev][iPoint][iProf]
// where iProf is Stats.nProfs[iLev][iPoint].
// Also bump Stats.nProfs[iLev][iPoint].

static void CollectProf (STATS &Stats,                      // io
                         int iLev, int iPoint,              // in
                         const Image &ScaledImg,
                         const SHAPE &ScaledShape,
                         const Mat   &Grads)
{
int iProf = Stats.nProfs[iLev][iPoint]; // start where we left off last time
unsigned ProfSpec = GetGenProfSpec(iLev, iPoint);
Mat *pProfs = &Stats.Profs[iLev][iPoint];   // profs of current point

if (IS_2D(ProfSpec))                    // two dimensional prof?
    {
    #if _MSC_VER // microsoft
        Get2dProf(pProfs->row(iProf),
                  ProfSpec, Grads, ScaledShape, iPoint,
                  0, 0, CONF_nProfWidth2d, CONF_SigmoidScale);
    #else   // TODO temporary var for gcc.  It's slow.
            // need to add conversion from GslMat::MatView to GslMat::Mat&
            // Compiler says "A non-const reference may only be bound to an lvalue"
        Vec Prof(pProfs->row(iProf));
        Get2dProf(Prof,
                  ProfSpec, Grads, ScaledShape, iPoint,
                  0, 0, CONF_nProfWidth2d, CONF_SigmoidScale);
        pProfs->row(iProf) = Prof;
    #endif
    }
else                                    // one dimensional prof
    {
    // this code supports only 1 one-dimensional profile per point
    ASSERT((ProfSpec & 0x00ffff00) == 0);

    PrepareProf1D(ScaledImg, ScaledShape, ProfSpec, gLandTab,
        iPoint, CONF_nProfWidth1d,
        0, CONF_fExplicitPrevNext);

    #if _MSC_VER // microsoft
        Get1dProf(pProfs->row(iProf), ProfSpec, ScaledImg, iPoint, 0);
    #else   // TODO temporary var for gcc
        Vec Prof(pProfs->row(iProf));
        Get1dProf(Prof, ProfSpec, ScaledImg, iPoint, 0);
        pProfs->row(iProf) = Prof;
    #endif
    }
Stats.nProfs[iLev][iPoint] = ++iProf;
}

//-----------------------------------------------------------------------------
static void CollectProfs (STATS &Stats,           // io
                          SH &Sh)                 // io: modified if nStandardFaceWidth != 0
{
clock_t StartTime = clock();
lprintf("Reading images ");
logprintf("\n");
const int nShapes = Sh.Shapes.size();
if (nShapes > 50)
    InitPacifyUser(nShapes);

// The outer loop is image-by-image so we only have to read each image once.
// This means that the Stats data structures have to be able to store
// all profiles --- a trade off of memory against execution speed.

for (int iShape = 0; iShape < nShapes; iShape++)
    {
    if (nShapes > 50)
        PacifyUser(iShape);

    SHAPE *pShape = &Sh.OrgShapes[iShape];
    Image Img;
    sLoadImageGivenDirs(Img,
        sGetBasenameFromTag(Sh.Tags[iShape]), Sh.sImageDirs, CONF_sShapeFile);

    // scale mean face width to CONF_nStandardFaceWidth

    const double FaceScale = GetFaceScale(*pShape, CONF_nStandardFaceWidth);
    *pShape *= FaceScale;
    ScaleImage(Img,
        iround(FaceScale * Img.width), iround(FaceScale * Img.height),
        CONF_fPrescaleBilinear);

    // The inner loop is level-by-level so we only have to reduce each image once

    for (int iLev = 0; iLev < CONF_nLevs; iLev++)
        {
        double PyrScale = GetPyrScale(iLev, CONF_PyrRatio);
        SHAPE ScaledShape(*pShape / PyrScale);
        Image ScaledImg;
        ReduceImageAssign(ScaledImg, Img, PyrScale, CONF_PyrReduceMethod);
        Mat Grads;
        const int nPoints = Sh.Shapes[Sh.iRefShape].nrows();
        InitGradsIfNeededForModelGeneration(Grads, ScaledImg, iLev, nPoints);

        for (int iPoint = 0; iPoint < nPoints; iPoint++)
            if (fMustCollectProf(Sh, iShape, iPoint, ScaledShape))
                CollectProf(Stats, iLev, iPoint,
                            ScaledImg, ScaledShape, Grads);

        if (CONF_fSynthEyePoints)
            SynthEyeProfs(Stats.Profs[iLev],
                Stats.nProfs[iLev], nPoints, iShape, iLev,
                Sh.fExtraLEyeLandsUsed, Sh.fExtraREyeLandsUsed, Sh.TagInts);
        }
    if (iShape % 30 == 0)
        ReleaseProcessor();     // give others a chance
    }
if (nShapes > 50)
    lprintf("0 ");  // finish pacifying
lprintf("[%.1f secs]\n", double(clock() - StartTime) / CLOCKS_PER_SEC);
}

//-----------------------------------------------------------------------------
// Return true if everything ok with the matrix Covar.
// Return false if something not good about Covar, also print reason.
// We use lprintf and not Warning below to get the line spacing right.

static bool fGoodCovar (const Mat &Covar,   // in: all
                        const char sMsg[],
                        int iLev,
                        int iPoint)
{
bool fGood = false;

if (!Covar.fIeeeNormal())
    lprintf("\nWarning: iLev %d iPoint %d: %s mat not IEEE normal",
        iLev, iPoint, sMsg);

// we use FLT_MAX below as a convenient big number for
// checking valid inversion of matrix)

else if (Covar.maxAbsElem() > FLT_MAX)
    lprintf("\niWarning: Lev %d iPoint %d: %s mat elem too big",
        iLev, iPoint, sMsg);

else if (!fPosDef(Covar))               // will test for symmetry too
    lprintf("\niWarning: Lev %d iPoint %d: %s mat not positive definite",
        iLev, iPoint, sMsg);

else
    fGood = true;

return fGood;
}

//-----------------------------------------------------------------------------
// just a little helper for GenInvertedCovarMat, to issue a clear warnng msg

static void IssueNbrOfShapesWarning (const Mat &Profs, const unsigned ProfSpec)
{
static bool fWarned = false; // to prevent duplicate warnings

if (!fWarned)
    {
    if (IS_2D(ProfSpec))
        WarnWithNewLine("number of shapes %d is less than "
            "profile length %dx%d = %d, "
            "generating identity covars",
            Profs.nrows(),
            nGetProfWidth(Profs.ncols(), ProfSpec),
            nGetProfWidth(Profs.ncols(), ProfSpec),
            Profs.ncols());
    else
        WarnWithNewLine("number of shapes %d is less than "
            "profile length %d, "
            "generating identity covars",
            Profs.nrows(), Profs.ncols());

    fWarned = true;
    }
}

//-----------------------------------------------------------------------------
// This generates the inverted covariance matrix Covar from Profs and AvProfs.
// If that is not possible, we generate an identity matrix instead
// and print a warning message.

static void GenInvertedCovarMat (Mat &Covar,    // out
    const Mat &Profs, const Mat &AvProfs,       // in
    int nProfs, unsigned ProfSpec,              // in
    int iLev, int iPoint)                       // in
{
ASSERT(nProfs >= 1);

if (Profs.nrows() < Profs.ncols())
    {
    // not enough data, covar mat will be singular, so use ident mat instead

    IssueNbrOfShapesWarning(Profs, ProfSpec);
    Covar = IdentMat(Profs.ncols());
    }
else
    {
#if 1
    // calculate covar mat
    Vec AllOnes(Profs.nrows()); AllOnes.fill(1);
    Mat AvMat(AllOnes * AvProfs);
    Mat CenteredProfs = Profs - AvMat;
    Covar = (CenteredProfs.t() * CenteredProfs) / nProfs;
#else
    // old way of calculating covar mat, not numerically
    // stable, see e.g. "Numerical Recipes"
    Mat Covar = (Profs.t() * Profs - AvProfs.t() * AvProfs * nProfs) / nProfs;
#endif

    bool fGood = false;
    if (fGoodCovar(Covar, "", iLev, iPoint))
        {
        Covar.invertMe();
        fGood = true;
        }
    if (!fGood || !fGoodCovar(Covar, "inverted ", iLev, iPoint))
        {
        // replace bad inverted cov mat with identity mat

        lprintf(", replacing covar mat with identity matrix ");
        Covar.identityMe();
        }
    // Scale covariances so largest is 10000. This allows us to write
    // the covariances as integers (using "%.0f") in WriteCovarMats().
    // This reduces the size of the generated .asm file
    // with no measurable loss of landmark location accuracy.

    Covar /= (Covar.maxElem() / 10000);
    }
}

//-----------------------------------------------------------------------------
static void GenAvProf (Mat &AvProf,         // out: assumed all 0 on entry
                       const Mat &Profs,    // in
                       int nProfs)          // in
{
for (int iProf = 0; iProf < nProfs; iProf++)
    AvProf += Profs.row(iProf);             // add current row of Profs

AvProf /= nProfs;
}

//-----------------------------------------------------------------------------
// This generates the (inverted) Covar and AvProf for one landmark on one level

static void GenAsmStats (Mat &Covar, Mat &AvProf,       // out
                         const Mat &Profs,              // in
                         int nProfs, unsigned ProfSpec, // in
                         int iLev, int iPoint)          // in
{
GenAvProf(AvProf, Profs, nProfs);
GenInvertedCovarMat(Covar, Profs, AvProf, nProfs, ProfSpec, iLev, iPoint);
if (IS_2D(ProfSpec) && CONF_nTrimCovar)
    IterativeTrimMat(Covar, CONF_nTrimCovar, ProfSpec);
}

//-----------------------------------------------------------------------------
// Generate profile stats to matrices Stats.AvProfs, Stats.AvProfs, Stats.Covars

static void GenProfStats (STATS &Stats, // io
        SH &Sh,                         // io: modified if nStandardFaceWidth != 0
        char sProfSpecs[])              // out: for user info only
{
int iPoint;

// Phase1: allocate space in Stats

const int nPoints = Sh.Shapes[Sh.iRefShape].nrows();
Check_nSubProfsIsOne(nPoints);
AllocStats(Stats, sProfSpecs, Sh);
lprintf("ProfSpecs %s\n", sProfSpecs);

// Phase2: collect profiles i.e. fill in Stats.Profs

CollectProfs(Stats, Sh);

// Phase3: resize the profile data to the size we are actually using.
// We over-allocated (to nMaxProfsPerPoint) in AllocStats() above.

for (int iLev = 0; iLev < CONF_nLevs; iLev++)
    for (iPoint = 0; iPoint <  nPoints; iPoint++)
        {
        const int nCols = Stats.Profs[iLev][iPoint].ncols();
        Stats.Profs[iLev][iPoint].dimKeep(Stats.nProfs[iLev][iPoint], nCols);
        }

// Phase4: calculate ASM summary stats i.e. fill
// in Stats.AvProfs and Stats.Covars from Stats.Profs

clock_t StartTime = clock();
lprintf("Generating stats ");
const int nShapes = Sh.Shapes.size();
if (nShapes > 50)
    InitPacifyUser(nPoints);
for (iPoint = 0; iPoint < nPoints; iPoint++)
    {
    if (nShapes > 50)
        PacifyUser(iPoint);
    for (int iLev = 0; iLev < CONF_nLevs; iLev++)
        {
        unsigned ProfSpec = GetGenProfSpec(iLev, iPoint);
        Mat *pCovar = &Stats.Covars[iLev][iPoint];

        GenAsmStats(*pCovar, Stats.AvProfs[iLev][iPoint],
                    Stats.Profs[iLev][iPoint],
                    Stats.nProfs[iLev][iPoint],
                    ProfSpec, iLev, iPoint);
        }
    if (CONF_nLev2d >= 0)
        ReleaseProcessor(); // give others a chance (2D profs are slow)
    }
if (nShapes > 50)
    lprintf("0 ");  // finish pacifying
lprintf("[%.1f secs]\n", double(clock() - StartTime) / CLOCKS_PER_SEC);
}

//-----------------------------------------------------------------------------
// Write covariance matrices for all points for one level iLev

static void WriteCovarMats (const STATS &Stats,                 // all in
                            int iLev, int nPoints,
                            const char sFile[], FILE *pAsmFile)
{
for (int iPoint = 0; iPoint < nPoints; iPoint++)
    {
    ASSERT(Stats.nProfs[iLev][iPoint] > 0);
    const Mat *pCovar = &Stats.Covars[iLev][iPoint];
    char sTag[SLEN];
    sprintf(sTag, "Covar Lev %d Point %d", iLev, iPoint);
    // see GenInvertedCovarMat for a comment on why we use "%.0f"
    pCovar->write(sFile, pAsmFile, "%.0f", sTag);
    }
}

//-----------------------------------------------------------------------------
// Write profiles for all points for one level iLev

static void WriteProfiles (const STATS &Stats, int iLev, int nPoints,
                           const char sFile[], FILE *pFile)
{
for (int iPoint = 0; iPoint < nPoints; iPoint++)
    {
    ASSERT(Stats.nProfs[iLev][iPoint] > 0);
    char sTag[SLEN]; sprintf(sTag, "Lev %d Prof %d", iLev, iPoint);
    Stats.AvProfs[iLev][iPoint].write(sFile, pFile, "%g", sTag);
    }
}

//-----------------------------------------------------------------------------
static void WriteProfSpecs (int iLev, int nPoints, FILE *pFile)
{
// for ProfSpecs we use the same layout as a Mat (but this is unsigned hex data)

Fprintf(pFile, "{ %d 1\n", nPoints);
for (int iPoint = 0; iPoint < nPoints; iPoint++)
    Fprintf(pFile, "%x\n", GetGenProfSpec(iLev, iPoint));
Fprintf(pFile, "}\n");
}

//-----------------------------------------------------------------------------
// Create a new .asm file i.e. write results to disk

static void WriteAsmFile (const char sFile[],           // in
                          const Vec &EigVals,           // in
                          const Mat &EigVecs,           // in
                          const STATS &Stats,           // in
                          const SH &Sh,                 // in
                          const char sProfSpecs[],      // in
                          int argc,                     // in
                          const char **argv)            // in
{
lprintf("Writing %s ", sFile);
FILE *pFile = Fopen(sFile, "w");    // will give a msg and exit if can't open

// first line of .asm file: ASM2 [filename tasm version 0.0 time]

time_t ltime; time(&ltime);
char *sTime = ctime(&ltime);
sTime[strlen(sTime)-1] = 0; // remove \n
Fprintf(pFile, "ASM%d [%s generated by tasm %s %s]\n# ",
    ASM_FILE_VERSION, sFile, TASM_VERSION, sTime);

PrintCmdLine(pFile, argc, argv);

// Parameters that the search routines need to know about
// These are in the format required by ReadConfTab in readconf.cpp

const int nPoints = Sh.Shapes[Sh.iRefShape].nrows();

Fprintf(pFile, "{\n");
Fprintf(pFile, "nPoints %d\n", nPoints);
Fprintf(pFile, "fXm2vts %d\n", CONF_fXm2vts);
Fprintf(pFile, "nLevs %d\n", CONF_nLevs);
Fprintf(pFile, "PyramidRatio %g\n", double(CONF_PyrRatio));
Fprintf(pFile, "nPyramidReduceMethod %d\n", CONF_PyrReduceMethod);
Fprintf(pFile, "fExplicitPrevNext %d\n", CONF_fExplicitPrevNext);
Fprintf(pFile, "NormalizedProfLen %g\n", CONF_NormalizedProfLen);
Fprintf(pFile, "nStandardFaceWidth %d\n", CONF_nStandardFaceWidth);
Fprintf(pFile, "fBilinearRescale %d\n", CONF_fPrescaleBilinear);
Fprintf(pFile, "nTrimCovar %d\n", CONF_nTrimCovar);
Fprintf(pFile, "SigmoidScale %g\n", CONF_SigmoidScale);
Fprintf(pFile, "}\n");

// Following go into the ASM file as comments because they are
// unneeded by stasm, but useful for debugging

Fprintf(pFile, "# ShapeFile %s\n", CONF_sShapeFile);
Fprintf(pFile, "# ImageDirs %s\n", Sh.sImageDirs);
Fprintf(pFile, "# nMaxShapes %d\n", CONF_nMaxShapes);
Fprintf(pFile, "# nShapes %d\n", Sh.Shapes.size());
Fprintf(pFile, "# sTagRegex \"%s\"\n", CONF_sTagRegex);
Fprintf(pFile, "# AttrMask0 0x%4.4x [%s]\n",
                CONF_AttrMask0, sGetAtFaceString(CONF_AttrMask0));
Fprintf(pFile, "# AttrMask1 0x%4.4x [%s]\n",
                CONF_AttrMask1, sGetAtFaceString(CONF_AttrMask1, true));
Fprintf(pFile, "# nLev2d %d\n", CONF_nLev2d);
Fprintf(pFile, "# nWhich2d %d\n", CONF_nWhich2d);
Fprintf(pFile, "# ProfType 0x%4.4x\n", CONF_ProfType);
Fprintf(pFile, "# ProfType2d 0x%4.4x\n", CONF_ProfType2d);
Fprintf(pFile, "# ProfSpecs %s\n", sProfSpecs);
Fprintf(pFile, "# fSynthEyePoints %d\n", CONF_fSynthEyePoints);
Fprintf(pFile, "# fTasmSkipIfNotInShapeFile %d\n", CONF_fTasmSkipIfNotInShapeFile);
Fprintf(pFile, "# iRefShape %d\n", Sh.iRefShape);
Fprintf(pFile, "# nSeed_ShapeNoise %d\n", CONF_nSeed_ShapeNoise);
Fprintf(pFile, "# nSeed_SelectShapes %d\n", CONF_nSeed_SelectShapes);
Fprintf(pFile, "# MaxShapeAlignDist %g\n", (double)CONF_MaxShapeAlignDist);
Fprintf(pFile, "# nProfWidth1d %d\n", CONF_nProfWidth1d);
Fprintf(pFile, "# nProfWidth2d %d\n", CONF_nProfWidth2d);
Fprintf(pFile, "# ShapeNoise %g\n", (double)CONF_ShapeNoise);
Fprintf(pFile, "# xRhsStretchShape %g\n", (double)CONF_xRhsStretchShape);
Fprintf(pFile, "# xStretchShape %g\n", (double)CONF_xStretchShape);
Fprintf(pFile, "# fUnobscuredFeats %d\n\n", CONF_fUnobscuredFeats);

EigVals.write(sFile, pFile, "%g", "EigVals");
EigVecs.write(sFile, pFile, "%g", "EigVecs");
Sh.AvShape.write(sFile, pFile, "%7.1f", "AvShape");
Sh.VjAv.write(sFile, pFile, "%7.1f", "VjAv");
Sh.RowleyAv.write(sFile, pFile, "%7.1f", "RowleyAv");

for (int iLev = CONF_nLevs-1; iLev >= 0; iLev--)
    {
    Fprintf(pFile, "Lev %d\n", iLev);
    Fprintf(pFile, "\"ProfSpecs Lev %d\"\n", iLev, nPoints);
    WriteProfSpecs(iLev, nPoints, pFile);

    WriteCovarMats(Stats, iLev, nPoints, sFile, pFile);
    WriteProfiles(Stats, iLev, nPoints, sFile, pFile);
    }
fclose(pFile);
lprintf("\n");
}

//-----------------------------------------------------------------------------
static int nUsedLandmarks (const Mat &Shape)
{
const int nPoints = Shape.nrows();
int nUsed = 0;

for (int iPoint = 0; iPoint < nPoints; iPoint++)
    if (fPointUsed(Shape, iPoint))
        nUsed++;

return nUsed;
}

//-----------------------------------------------------------------------------
// This inits (in the Sh parameter):
//             OrgShapes Shapes Tags TagInts sImageDirs
//             iRefShape nUsedLandmarks
//             fExtraLEyeLandsUsed fExtraREyeLandsUsed
//
// It adds synthesized eye points to OrgShapes if CONF_fSynthEyePoints.
// On return Shapes is the same as OrgShapes.
//
// We only read matrices where (AttrMask & CONF_Mask1) == CONF_AttrMask0
// (AttrMask is hex part of tag string) and CONF_sTagRegex matches the shape
// name in the tag string.
//
// We read all shapes matching the above criteria if CONF_nMaxShapes==0
// else we read a max of CONF_nMaxShapes.

static void ReadTrainingShapes (SH &Sh)    // out
{
ReadSelectedShapes(Sh.OrgShapes, Sh.Tags, Sh.sImageDirs,
    CONF_sShapeFile, CONF_nMaxShapes,
    CONF_sTagRegex, CONF_AttrMask0, CONF_AttrMask1);

if (CONF_fSynthEyePoints)
    {
    if (Sh.OrgShapes[0].nrows() != 84)
        Err("fSynthEyePoints can be used only with 84 point shapes");

    InitExtraEyeLandsUsed(Sh.fExtraLEyeLandsUsed, Sh.fExtraREyeLandsUsed,
                      Sh.OrgShapes);
    SynthEyePoints(Sh.OrgShapes);
    }

// get the index of the shape to use a reference for aligning shapes
Sh.iRefShape = iGetRefShapeIndex(Sh.Tags, CONF_AttrMask0, CONF_AttrMask1,
                                 CONF_sShapeFile);

const int nWantedPoints = Sh.OrgShapes[Sh.iRefShape].nrows();

lprintf("Reference shape %s has %d landmarks\n",
        Sh.Tags[Sh.iRefShape].c_str(), nWantedPoints);

const int nShapes = Sh.OrgShapes.size();
Sh.TagInts.resize(nShapes);
Sh.Shapes.resize(nShapes);
Sh.nUsedLandmarks.resize(nShapes);

vec_int nUnusedLandmarks(MAX_NBR_LANDMARKS, 0);

for (int iShape = 0; iShape < nShapes; iShape++)
    {
    Sh.Shapes[iShape] = Sh.OrgShapes[iShape];
    Sh.nUsedLandmarks[iShape] = nUsedLandmarks(Sh.Shapes[iShape]);

    const char *sTag = Sh.Tags[iShape].c_str();
    const char *s = sGetBasenameFromTag(sTag);
    ASSERT(isprint(s[0]) && !isspace(s[0]));

    unsigned temp;
    if (1 != sscanf(Sh.Tags[iShape].c_str(), "%x", &temp))
        Err("can't convert tag %s in %s to a hex number", sTag, CONF_sShapeFile);
    Sh.TagInts[iShape] = temp;
    }
}

//-----------------------------------------------------------------------------
// Make sure that the values read from sConfFile are valid.
// This doesn't check everything, just some of the common errors.
// Values that are invalid but are not checked here will cause
// ASSERT fails later (always?).

static void CheckConfParams (const SH &Sh, const char sConfFile[])
{
if (CONF_fXm2vts)
    {
    const int nPoints = Sh.Shapes[Sh.iRefShape].nrows();

    if (nPoints < 68)
        Err("fXm2vts is true but nPoints %d < 68\n"
            "       Check %s",
            nPoints, sConfFile);

    if (nPoints > ngElemsLandTab)
        Err("fXm2vts is true but "
            "nPoints %d > ngElemsLandTab %d\n"
            "       Check %s",
            nPoints, ngElemsLandTab, sConfFile);
    }
else
    {
    if (CONF_fExplicitPrevNext)
        Err("fExplicitPrevNext can only be true when fXm2vts is true "
            "(because it needs the gLandTab table)\n"
            "       Check %s", sConfFile);

    if (CONF_fUnobscuredFeats)
        Err("fUnobscuredFeats can only be true when fXm2vts is true "
            "(because it needs the gLandTab table)\n"
            "       Check %s", sConfFile);

    if (CONF_nWhich2d != GEN2D_All)
        Err("nWhich2d must be 0 when fXm2vts is false "
            "(because it needs the gLandTab table)\n"
            "       Check %s", sConfFile);
    }
// you can lift the following limitation by extending Normalize1d and friends

if (CONF_ProfType != (PROF_Grad|PROF_Flat))
    Err("tasm supports only CONF_ProfType == 0x%x, you have 0x%x\n"
        "       Check %s",
        (PROF_Grad|PROF_Flat), CONF_ProfType, sConfFile);

// you can lift the following limitation by extending Get2dProfFromGradMats and friends

if (CONF_ProfType2d != (PROF_WindowEquallyWeighted|PROF_SigmAbsSum|PROF_GradBelowRight))
    Err("tasm supports only CONF_ProfType2d == 0x%x, you have 0x%x\n"
        "       Check %s",
        (PROF_WindowEquallyWeighted|PROF_SigmAbsSum|PROF_GradBelowRight),
        CONF_ProfType2d, sConfFile);
}

//-----------------------------------------------------------------------------
int main (int argc, const char *argv[])
{
clock_t StartTime = clock();

int argc1 = argc;                       // save for PrintCmdLine()
const char **argv1 = argv;

char sConfFile[SLEN]; sConfFile[0] = 0; // input .conf file
char sAsmFile[SLEN];                    // name of the generated .asm file
strcpy(sAsmFile, "temp.asm");           // default .asm filename

SH Sh;
const char *sUseQForHelp = "Use tasm -? for help.";

while (--argc > 0 && (*++argv)[0] == '-')
    {
    const char c = *(*argv + 1);
    if (*(*argv + 2))
        Err("extra character '%c' after -%c.  %s",
            *(*argv + 2), c, sUseQForHelp);
    switch (c)
        {
        case 'o':           // output file
            argv++;
            argc--;
            if (argc < 1 || *(*argv) == '-')
                Err("-o needs an argument.  %s", sUseQForHelp);
            strcpy(sAsmFile, *argv);
            break;
        case '?':           // help
            lprintf(sgUsage);
            exit(1);
        default:
            Err("unrecognized option %s.  %s", *argv, sUseQForHelp);
        }
    }
if (argc == 0)  // argv should now point to FILE.conf
    Err("no config file.  %s", sUseQForHelp);
if (argc > 1)
    Err("too many arguments.  %s", sUseQForHelp);

// after pOpenLogFile(), lprintfs will go to the screen and to the log file
pgLogFile = pOpenLogFile("tasm.log", VERBOSE);
logprintf("tasm %s\n", TASM_VERSION);
PrintCmdLine(pgLogFile, argc1, argv1);

// read global "CONF_" params from .conf file

strcpy(sConfFile, *argv);
ReadConfFile(TasmConfTab, NELEMS(TasmConfTab), sConfFile, "TASM");

// get shapes from shape file

ReadTrainingShapes(Sh);
CheckConfParams(Sh, sConfFile);
if (CONF_fXm2vts)
    InitLandTab(Sh.Shapes[Sh.iRefShape].nrows());

// build shape model i.e. calculate EigVals, EigVecs, AvShape
// (this also aligns Sh.Shapes)

const int nPoints = Sh.Shapes[Sh.iRefShape].nrows();
Vec EigVals(2 * nPoints);
Mat EigVecs(2 * nPoints, 2 * nPoints);

GenShapeModel(EigVals, EigVecs, Sh.AvShape, Sh.Shapes,
              Sh.iRefShape, Sh.Tags, Sh.nUsedLandmarks);

// scale AvShape face width to CONF_nStandardFaceWidth

Sh.AvShape *= GetFaceScale(Sh.AvShape, CONF_nStandardFaceWidth);

// generate Viola Jones and Rowley detector average shapes

Sh.VjAv.dimClear(nPoints, 2);
Sh.RowleyAv.dimClear(nPoints, 2);
char sDataDir[SLEN]; // data dir for face detector data files
GetDataDir(sDataDir, *argv1);
GenDetAvShapes(Sh.VjAv, Sh.RowleyAv,
               CONF_sShapeFile, CONF_fTasmSkipIfNotInShapeFile,
               Sh.OrgShapes, Sh.Tags,
               Sh.nUsedLandmarks, sDataDir);

// generate profile statistics (results go into Stats)

STATS Stats;
char sProfSpecs[1000];
GenProfStats(Stats, Sh, sProfSpecs);

#if GEN_RMAT // generate matrices for analysis with R
GenMatricesForR(Sh, Stats);
#endif

// write results to disk

WriteAsmFile(sAsmFile, EigVals, EigVecs,
             Stats, Sh, sProfSpecs, argc1, argv1);

lprintf("Generated %s [total time %.1f secs]\n",
        sAsmFile, double(clock() - StartTime) / CLOCKS_PER_SEC);

ShutdownStasm();
return 0;
}
