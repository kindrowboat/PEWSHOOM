// $imfile.hpp 3.0 milbo$ routines for image files

#if !defined(imfile_hpp)
#define imfile_hpp

const char *
sLoadImage(Image *pImg, RgbImage *pRgbImg,              // out
           const char sFile[], bool fVerbose=false,     // in
           bool fExitOnErr=true, bool fRemovePad=true); // in

const char *
sLoadImage(Image &Img,                                  // out
           const char sFile[], bool fVerbose=false,     // in
           bool fExitOnErr=true);                       // in

const char *
sLoadImage(RgbImage &RgbImg,                            // out
           const char sFile[], bool fVerbose=false,     // in
           bool fExitOnErr=true, bool fRemovePad=true); // in

#endif // imfile_hpp
