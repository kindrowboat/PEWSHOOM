// $mouth.hpp 3.0 milbo$ landmark tables for mouth shapes

#if !defined(mouth_hpp)
#define mouth_hpp

static const int iXm2vtsToMouth[] = {
    MLMouthCorner,      // 48
    MMouth49,           // 49
    MMouth50,           // 50
    MMouthTopOfTopLip,  // 51
    MMouth52,           // 52
    MMouth53,           // 53
    MRMouthCorner,      // 54
    MMouth55,           // 55
    MMouth56,           // 56
    MMouthBotOfBotLip,  // 57
    MMouth58,           // 58
    MMouth59,           // 59
    MMouth60,           // 60
    MMouthTopOfBotLip,  // 61
    MMouth62,           // 62
    MMouth63,           // 63
    MMouth64,           // 64
    MMouth65,           // 65
    MMouthBotOfTopLip   // 66
};
static const int iMouthToXm2vts[] = {
    -1,     // MLTemple         0 landmark names in internal numbering scheme
    -1,     // MLJaw1           1 left right are wrt the viewer, not the subject
    -1,     // MLJaw2           2
    -1,     // MLJaw3_MouthLine 3 mouth line on jaw
    -1,     // MLJaw4           4
    -1,     // MLJaw5           5
    -1,     // MLJaw6           6
    -1,     // MTipOfChin       7
    -1,     // MRJaw6           8
    -1,     // MRJaw5           9
    -1,     // MRJaw4           10
    -1,     // MRJaw3_MouthLine 11 mouth line on jaw
    -1,     // MRJaw2           12
    -1,     // MRJaw1           13
    -1,     // MRTemple         14
    -1,     // MROuterEyeBrow   15
    -1,     // MROuterTopEyeBrow 16
    -1,     // MRInnerTopEyeBrow 17
    -1,     // MRInnerEyeBrow   18
    -1,     // MPoint19         19
    -1,     // MPoint20         20
    -1,     // MLOuterEyeBrow   21
    -1,     // MLOuterTopEyeBrow 22
    -1,     // MLInnerTopEyeBrow 23
    -1,     // MLInnerEyeBrow   24
    -1,     // MPoint25         25
    -1,     // MPoint26         26
    -1,     // MLEyeOuter       27
    -1,     // MLEyeTop         28
    -1,     // MLEyeInner       29
    -1,     // MLEyeBottom      30
    -1,     // MLEye            31 pupil
    -1,     // MREyeOuter       32
    -1,     // MREyeTop         33
    -1,     // MREyeInner       34
    -1,     // MREyeBottom      35
    -1,     // MREye            36 pupil
    -1,     // MLNoseTop        37
    -1,     // MLNoseMid        38
    -1,     // MLNoseBot0       39
    -1,     // MLNoseBot1       40
    -1,     // MNosebase        41
    -1,     // MRNoseBot1       42
    -1,     // MRNoseBot0       43
    -1,     // MRNoseMid        44
    -1,     // MRNoseTop        45
    -1,     // MLNostril        46
    -1,     // MRNostril        47
     0,     // MLMouthCorner    48
     1,     // MMouth49         49
     2,     // MMouth50         50
     3,     // MMouthTopOfTopLip 51
     4,     // MMouth52         52
     5,     // MMouth53         53
     6,     // MRMouthCorner    54
     7,     // MMouth55         55
     8,     // MMouth56         56
     9,     // MMouthBotOfBotLip 57
    10,     // MMouth58         58
    11,     // MMouth59         59
    12,     // MMouth60         60
    13,     // MMouthTopOfBotLip 61
    14,     // MMouth62         62
    15,     // MMouth63         63
    16,     // MMouth64         64
    17,     // MMouth65         65
    18,     // MMouthBotOfTopLip 66
    -1,     // MNoseTip         67
// TODO following names are correct for 84 point shapes but misleading for muct3 and muct4 shapes
    -1,     // MLEye0           68 extra points beyond XM2VTS
    -1,     // MLEye1           69
    -1,     // MLEye2           70
    -1,     // MLEye3           71
    -1,     // MLEye4           72
    -1,     // MLEye5           73
    -1,     // MLEye6           74
    -1,     // MLEye7           75
    -1,     // MREye0           76 REye points synthesized from extra LEye points
    -1,     // MREye1           77
    -1,     // MREye2           78
    -1,     // MREye3           79
    -1,     // MREye4           80
    -1,     // MREye5           81
    -1,     // MREye6           82
    -1      // MREye7           83
};

#endif // mouth_hpp
