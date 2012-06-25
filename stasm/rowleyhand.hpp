// $rowleyhand.hpp 3.0 milbo$ hand tuned startshape from Rowley detector

typedef enum eDetLandmarks
    {
    DET_TopLeft,   // global face position
    DET_BotRight,  // global face position
    DET_LEye,      // left eye position
    DET_REye       // right eye position
    }
eDetLandmarks;

SHAPE DetParamsToShapeOld(const DET_PARAMS &DetParams);

void
AlignToHandRowley(SHAPE &StartShape,            // out
                  const DET_PARAMS &DetParams,  // in
                  const SHAPE &MeanShape);      // in
