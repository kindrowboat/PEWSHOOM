// $startshape.hpp 3.0 milbo$ routines for finding start shape for an ASM search

#if !defined(startshape_hpp)
#define startshape_hpp

// The constant 200 is arbitrary, except that the value used by stasm
// must match that used by tasm.  Using 200 instead of say, 1, means that
// the detector average face is displayable at a decent size which is
// useful for debugging.

static const int DET_FACE_WIDTH = 200;

// Parameters returned by the face detectors.
// x and y and eye positions are cartesian coords with center of image at 0,0

typedef struct DET_PARAMS {
    double x, y;            // center of detector shape
    double width, height;   // width and height of detector shape
    double lex, ley;        // center of left eye
    double rex, rey;        // ditto for right eye
                            // right and left are w.r.t. the viewer.
} DET_PARAMS;

extern int CONF_nRowleyMethod;
extern int CONF_nVjMethod;
extern double CONF_RowleyScale;
extern double CONF_VjScale;
extern bool CONF_fRowleyUseEvenIfEyeMissing;
extern bool CONF_fVjUseEvenIfEyeMissing;
extern bool CONF_fRowleySynthMissingEye;
extern bool CONF_fVjSynthMissingEye;

void AlignToDetFrame(SHAPE &Shape,                  // io
                     const DET_PARAMS &DetParams);  // in

void VecToDetParams(DET_PARAMS &DetParams, const Mat &v);

SHAPE DetParamsToShape(const DET_PARAMS &DetParams); // in

bool fFindDetParams(DET_PARAMS &DetParams,  // out
                    const char sPath[],     // in: image path
                    unsigned DetAttr,       // in: specifies which face detector
                    const char sDataDir[],  // in: for face det data files
                    bool fIssueWarnings);   // in: true to issue warnings if needed

int nGetDetParams(DET_PARAMS &DetParams,     // out
                const char sImage[],         // in: image name
                unsigned Attr,               // in: specifies which face detector
                const char sShapeFile[],     // in: shape file
                const char sDataDir[],       // in: for face det data files
                bool fUseDirsInShapeFile,    // in: use dir path in shape file?
                bool fSkipIfNotInShapeFile,  // in:
                bool fIssueWarnings);        // in: true to issue warnings if needed

bool
fGetStartShape(
        SHAPE &StartShape,              // out: the start shape we are looking for
        DET_PARAMS &DetParams,          // out: informational only
        const char sImage[],            // in
        const SHAPE &MeanShape,         // in
        unsigned DetAttr,               // in: specifies which face detector
        const SHAPE &DetAv,             // in: either VjAv or RowleyAv
        const char sShapeFile[],        // in: if not NULL, look for face detector
                                        //     shape for sImage here first
        const char sDataDir[],          // in: for face detector data files
        const bool fSkipIfNotInShapeFile,
        bool fIssueWarnings);           // in: true to issue warnings if needed

void GetStartShapeFromPreviousSearch(SHAPE &StartShape,     // out
        const SHAPE &PrevShape,         // in: combined shape from previous search
        const SHAPE &MeanShape);        // in

void PossiblyIssueDetWarning(bool fFound,                   // in: all
                             unsigned DetAttr,
                             const DET_PARAMS &DetParams);
#endif // startshape_hpp
