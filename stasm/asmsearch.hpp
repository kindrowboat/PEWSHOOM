// $asmsearch.hpp 3.0 milbo$ search routines for active shape models

#if !defined(asmsearch_hpp)
#define asmsearch_hpp

static const int MAX_NBR_LEVS     = 6;   // max levs in image pyr
static const int ASM_FILE_VERSION = 1;   // ASM file version

typedef struct ASM_LEVEL_DATA     // ASM data for one level: level 0 is full size
    {
    int               nEigs;      // nEigs for LimitB()
    int               nEigsFinal; // nEigs for for the final search iteration
    double            BMax;       // for LimitB()
    double            BMaxFinal;
    int               nMaxSearchIters;
    int               nQualifyingDisp;
    SHAPE             MeanShape;  // FileMeanShape, scaled for this level

    vector<Mat>       Covars;     // vec [nPoints] of INVERSE of prof covar
                                  // mat, each is mat nPoints x nPoints

    vector<SparseMat> SparseCovars; // same as Covars, but sparse arrays

    vector<Vec>       Profs;      // vec [nPoints] of landmark profile
                                  // row vecs, each is 1 x nPoints

    vector<unsigned>  ProfSpecs;  // see PROF_ defs in prof.hpp
    Vec               EigVals;    // n x 1 vector, EigVals scaled for this level
    }
ASM_LEVEL_DATA;

typedef struct ASM_MODEL       // an ASM model
    {
    char   sFile[SLEN];
    int    iModel;             // used if models are stacked, 1st model is 0
    int    nPoints;            // number of landmarks
    int    nStartLev;          // start the search at this level in image pyramid
    double PyrRatio;
    int    PyrReduceMethod;    // one of IM_NEAREST_PIXEL IM_BILINEAR IM_AVERAGE_ALL
    int    nTrimCovar;         // covariance trim
    double SigmoidScale;       // specify how curvy profile equalization sigmoid is
    int    nStandardFaceWidth; // if >0 then prescale input face to this size
    int    nPixSearch;         // max dist from landmark when searching, in pixels
    int    nPixSearch2d;       // ditto but for for 2d profiles
    bool   fExplicitPrevNext;  // use iPrev and iNext in LandTab? see landmarks.hpp
    bool   fBilinearRescale;
    SHAPE  FileMeanShape;      // mean shape read from .asm file
    SHAPE  VjAv;
    SHAPE  RowleyAv;
    Mat    EigVecs;            // n x n matrix
    Mat    EigInverse;         // EigVecs inverted
    ASM_LEVEL_DATA AsmLevs[MAX_NBR_LEVS]; // model for each pyramid level
    }
ASM_MODEL;

SHAPE                               // results returned as a SHAPE
AsmSearch(
    SHAPE &StartShape,              // out: start shape returned in here
    DET_PARAMS &DetParams,          // out: face detector parameters
    double &MeanTime,               // out: mean time per image (face det failed excluded)
    const RgbImage &RgbImg,         // in: find face features in this image
    const char sImage[],            // in: file path for RgbImg, for err msgs
    const bool fRowley=false,       // in: true to use VJ detector, else Rowley
    const char sConfFile0[]="../data/mu-68-1d.conf", // in: 1st config filename
    const char sConfFile1[]="../data/mu-76-2d.conf", // in: 2nd config filename
    const char sDataDir[]="../data",// in: data directory
    const char sShapeFile[]=NULL,   // in: if not NULL then use face detector in here
    bool fIssueWarnings=true);      // in: true to issue warnings if needed

#endif // asmsearch_hpp
