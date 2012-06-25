// $follow.hpp 3.0 milbo$
// Derived from code by Henry Rowley http://vasc.ri.cmu.edu/NNFaceDetector.

#ifndef follow_hpp
#define follow_hpp

bool fFindNewLocation(int *newX, int *newY, int *news,
                      int numScales, Image ImagePyramid[], Image &FaceMask,
                      int x, int y, int scale,
                      int dx, int dy, int ds, int step, int iNet);

#endif // follow_hpp
