// $find.hpp 3.0 milbo$
// Derived from code by Henry Rowley http://vasc.ri.cmu.edu/NNFaceDetector.

#ifndef find_hpp
#define find_hpp

int Track_FindAllFaces(DET_PARAMS *ppDetParams[],                // out
                       const Image &Img, const char sDataDir[]); // in

void
FindEyesGivenVjFace(DET_PARAMS &DetParams,  // io: on entry has VJ face box
                    const Image &Img,       // in
                    const char sDataDir[]); // in

#endif // find_hpp
