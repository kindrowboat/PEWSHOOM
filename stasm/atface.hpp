// $atface.hpp 3.0 milbo$ face attributes

// Face attribute bits, used in first part of string preceding each shape
// array (before filename) in shape file e.g. "0002 B0002_01.pgm" means
// that the face in image B0002_01.pgm is wearing glasses.
//
// The same defines are used to specify attributes of individual landmarks
// in landmarks.hpp:gLandTab e.g. FA_Glasses means that this landmark is
// obscured if the subject is wearing glasses
//
// FA_BadImage marking is subjective and not entirely consistent.  I just
// wanted to weed out images that seemed likely to confuse the classifier
// during training.  Certainly all _really_ bad images are marked FA_BadImage;
// marking is inconsistent only on marginally bad images.
//
// FA_NnFailed means that the Rowley neural net detector did not find one
// face and two eyes in the image.  Typically this means that the face is
// of poor quality or is wearing glasses. ("Nn" stands for "neural net").

#if !defined(atface_hpp)
#define atface_hpp

#define FA_BadImage   0x01       // image is "bad" in some way (blurred, face tilted, etc.)
#define FA_Glasses    0x02       // face is wearing specs
#define FA_Beard      0x04       // beard including possible mustache
#define FA_Mustache   0x08       // mustache but no beard occluding chin or cheeks
#define FA_Obscured   0x10       // faces is obscured e.g. by subject's hand
#define FA_EyesClosed 0x20       // eyes closed (partially open is not considered closed)
#define FA_Expression 0x40       // non-neutral expression on face
#define FA_NnFailed   0x80       // Rowley search failed (does not return 1 face with 2 eyes)
#define FA_Synthesize 0x100      // synthesize eye points from twin landmark
#define FA_VjFailed   0x200      // Viola Jones detector failed (no face found)
#define FA_ViolaJones 0x1000     // Viola Jones detector results
#define FA_Rowley     0x2000     // Rowley detector results

static const char * const sgFaceAttr[] =
    {
    "BadIm",        // 0001 FA_BadImage
    "Glasses",      // 0002 FA_Glasses
    "Beard",        // 0004 FA_Beard
    "Mustache",     // 0008 FA_Mustache

    "Obscured",     // 0010 FA_Obscured
    "EyesClosed",   // 0020 FA_EyesClosed
    "Expression",   // 0040 FA_Expression
    "NnFailed",     // 0080 FA_NnFailed

    "Synthesize",   // 0100 FA_Synthesize
    "VjFailed",     // 0200 FA_VjFailed
    "",             // 0400
    "",             // 0800 was FA_Remarked, is now available

    "ViolaJones",   // 1000 FA_ViolaJones
    "Rowley",       // 2000 FA_Rowley
    "",             // 4000
    "",             // 8000
    };

extern char *sGetAtFaceString(unsigned Attr, bool fSpecialHandlingForMask1=false);
extern char *sGetDetString(unsigned Attr);

#endif // atface_hpp
