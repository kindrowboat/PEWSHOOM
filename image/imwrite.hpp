// $imfile.hpp 3.0 milbo$ routines for image files

#if !defined(imwrite_hpp)
#define imwrite_hpp

void
WriteBmp(const Image &Img, const char sFile[], bool fVerbose=false);    // in

void
WriteBmp(const RgbImage &Img, const char sFile[], bool fVerbose=false); // in

#endif // imwrite_hpp
