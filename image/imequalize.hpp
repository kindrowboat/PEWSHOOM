// $imequalize.hpp 3.0 milbo$ image equalization and related routines

#if !defined(imequalize_hpp)
#define imequalize_hpp

static const int IM_STANDARD_EQUALIZE = 0;   // values for fQuick parameter
static const int IM_QUICK_EQUALIZE    = 1;

void EqualizeImage(Image &Img, bool fQuick=false);
void EqualizeMirrorImage(Image &Img);

void QuickEqualizeImage(Image &Img);    // io

void EqualizeRgbImage(RgbImage &Img);   // io

void QuickEqualizeRgbImage(RgbImage &Img,      // io
                           double EqAmount=1); // in: 0 to 1, amount of equalization

void EqualizeAndCorrectImage(Image &Img,                                // io
                    int nxMin, int nxMax, int nyMin, int nyMax,         // in: mask area in SHAPE coords
                    bool fVerbose=false);                               // in

#endif // imequalize_hpp
