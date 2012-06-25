// $violajones.hpp 3.0 milbo$ interface to OpenCV Viola Jones face detector

#if !defined(violajones_hpp)
#define violajones_hpp

void CloseViolaJones(void);

bool fFindViolaJonesFace(DET_PARAMS &DetParams,    // out
                         const Image &Img,         // in
                         const char sImage[],      // in
                         const char sDataDir[],    // in
                         const bool fEyes);        // in: find eyes too?

#endif // violajones_hpp
