// $asmsearch1.hpp 3.0 milbo$ search routines for active shape models

#if !defined(asmsearch1_hpp)
#define asmsearch1_hpp

extern ASM_MODEL gModels[];
extern int ngModels;
extern Vec b;

SHAPE ConformShapeToModelWithKeymarks(
                    Vec &b,                    // io
                    const SHAPE &Shape,        // in
                    const ASM_MODEL &Model,    // in
                    int iLev,                  // in
                    bool fShapeModelFinalIter, // in
                    const SHAPE &Keymarks);    // in: fixed landmarks

SHAPE                               // results returned as a SHAPE
AsmSearch1(
    const SHAPE *pStartShape,       // in: if not null then use, else search for start shape
    DET_PARAMS &DetParams,          // out: face detector parameters
    double &MeanTime,               // out: mean time per image (face det failed excluded)
    const RgbImage &RgbImg,         // in: find face features in this image
    const char sImage[],            // in: file path for RgbImg, for err msgs
    const bool fRowley,             // in: true to use Rowley detector, else VJ
    const char sConfFile0[]="../data/mu-68-1d.conf", // in: 1st config filename
    const char sConfFile1[]="../data/mu-76-2d.conf", // in: 2nd config filename
    const char sDataDir[]="../data",// in: data directory
    const char sShapeFile[]=NULL,   // in: if not NULL then use face detector in here
    bool fIssueWarnings=true,       // in: true to issue warnings if needed
    const SHAPE *pKeymarks=NULL);   // in: fixed landmarks, row set to 0,0 if not fixed

#endif // asmsearch1_hpp
