// $eyesynth.hpp 3.0 milbo$ functions to synthesize right eye points from left eye
// Forden jul 08
//-----------------------------------------------------------------------------

void SynthEyePoints(vec_SHAPE &Shapes);

void InitExtraEyeLandsUsed(vec_bool  &fExtraLEyeLandsUsed,   // out
                           vec_bool  &fExtraREyeLandsUsed,   // out
                           vec_SHAPE &OrgShapes);            // in

void SynthEyeProfs(vec_Mat &Profs,                      // io
                   vec_int &nProfs,                     // io
                   const int nPoints,                   // in
                   const int iShape,                    // in
                   const int iLev,                      // in
                   const vec_bool fExtraLEyeLandsUsed,  // in
                   const vec_bool fExtraREyeLandsUsed,  // in
                   const vec_int TagInts);              // in
