// $rowley.hpp 3.0 milbo$ interface between ASM software and Rowley face detector

#if !defined(rowley_hpp)
#define rowley_hpp

bool
fRowleyFindFace(DET_PARAMS &DetParams,      // out
                Image &Img,                 // in
                const char sImage[],        // in
                const char sDataDir[]);     // in

#endif // rowley_hpp
