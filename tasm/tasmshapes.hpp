// $tasmshapes.hpp 3.0 milbo$ shape model functions for tasm

#if !defined(tasmshapes_hpp)
#define tasmshapes_hpp

void GenShapeModel(Vec &EigVals,                     // out
                     Mat &EigVecs,                   // out
                     SHAPE &AvShape,                 // out
                     vec_SHAPE &Shapes,              // io: will be aligned
                     const int iRefShape,            // in
                     const vec_string &Tags,         // in
                     const vec_int &nUsedLandmarks); // in

#endif // tasmshapes_hpp
