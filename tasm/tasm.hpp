// $tasm.hpp 3.0 milbo$ externs for tasm.cpp
// Cape Town jun08

#if !defined(tasm_hpp)
#define tasm_hpp

typedef enum eGen2d // see CONF_nWhich2d and GetGenProfSpec
{
    GEN2D_All                 =  0,
    GEN2D_InternalExceptMouth =  1,
    GEN2D_Internal            =  2,
    GEN2D_Eyes                =  3, // original XM2VTS eye points
    GEN2D_EyesExt             =  4, // original plus extended XM2VTS eye points
    GEN2D_InternalExceptNose  =  5
} eGen2d;

extern int CONF_nLev2d;
extern eGen2d CONF_nWhich2d;
extern int CONF_nProfWidth1d;
extern int CONF_nProfWidth2d;
extern unsigned CONF_ProfType;
extern unsigned CONF_ProfType2d;
extern bool CONF_fExplicitPrevNext;
extern double CONF_MaxShapeAlignDist;
extern double CONF_xStretchShape;
extern double CONF_xRhsStretchShape;
extern double CONF_ShapeNoise;
extern int CONF_nSeed_ShapeNoise;

bool fPointObscured(const vec_int &TagInts, int iShape, int iPoint);

#endif // tasm_hpp
