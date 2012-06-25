// $libthatch.hpp 3.0 milbo$

#if !defined(libthatch_hpp)
#define libthatch_hpp

void Thatcherize (RgbImage &Img,        // io: thatcherize the face in this image
  const SHAPE &Shape,                   // in: facial landmarks (results of ASM search)
  double Expand=1,                      // in: adjust flipped area size
  bool   fMirrorMouth=false,            // in: mirror the mouth horizontally too
  bool   fRandomlyShiftFeatures=false,  // in: if true, randomly shift nose etc. slightly
  bool   fShowBoxes=false);             // in: show bounding boxes of thatcherized areas for debugging

void AddThirdEye(RgbImage &Img,         // io: the image with a face
  const SHAPE &Shape,                   // in: landmark results of ASM search
  double Expand=1,                      // in: adjust flipped area size
  bool   fShowBox=false);               // in: show bounding boxes of shifted areas for debugging

void FlipImage(RgbImage &Img,               // io:
               bool fFlipHorizontal=false,  // in:
               bool fFlipVertical=true);    // in:

#endif // libthatch_hpp
