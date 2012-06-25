// $shapemodel.hpp 3.0 milbo$

#if !defined(shapemodel_hpp)
#define shapemodel_hpp

double xShapeExtent(const SHAPE &Shape);
double yShapeExtent(const SHAPE &Shape);
bool fPointUsed(const SHAPE &Shape, int iPoint);
bool fPointUsed(const double x, const double y);

double PointToPointDist(const SHAPE &Shape1, int iPoint1,
                        const SHAPE &Shape2, int iPoint2);

void JitterPoints(SHAPE &Shape);

Mat AlignShape(SHAPE &Shape,                // io
                const SHAPE &AnchorShape,   // in
                const Vec *pWeights=NULL);  // in: can be NULL

Mat GetAlignTransform(const SHAPE &Shape,           // io
                      const SHAPE &AnchorShape);    // in

SHAPE
ConformShapeToModel(Vec &b,             // io
        const SHAPE &Shape,             // in
        const ASM_MODEL &Model,         // in
        int iLev,                       // in
        bool fShapeModelFinalIter);     // in

SHAPE TransformShape(const SHAPE &Shape, const Mat &TransformMat);

SHAPE TransformShape(const SHAPE &Shape,                // in
                     double x0, double y0, double z0,   // in
                     double x1, double y1, double z1);  // in

void TransformInPlace(SHAPE &Shape,                      // io
                      double x0, double y0, double z0,   // in
                      double x1, double y1, double z1);  // in

#endif // shapemodel_hpp
