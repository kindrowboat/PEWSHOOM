// $search.hpp 3.0 milbo$
// Derived from code by Henry Rowley http://vasc.ri.cmu.edu/NNFaceDetector.

#ifndef search_hpp
#define search_hpp

// callback function, used to save or otherwise process detection information

typedef void (*CALLBACK_FUN)(ClientData callbackData,
    Image *pImg, int x, int y, int width, int height,
    int iLev, double scale, double output);

class Detection : public ListNode<Detection>    // class to store detections
{
public:
    int x, y, scale;
    double output;
    Detection(int x1, int y1, int scale1, double output1)
        {
        x = x1; y = y1; scale = scale1; output = output1;
        }
};

void
SearchUme(Image &Img, Image *pLevMask, int iLev,
          CALLBACK_FUN callbackFunc, ClientData callbackData);

void
FuzzyVote2(int width, int height, int nLocFiles, List<Detection> Detections[],
    CALLBACK_FUN callbackFunc, ClientData callbackData,
    int spread, int search,
    bool fCollapseNearbyOverlappingDetections,
    bool fRemoveOverlappingImages,
    bool fFilterOdd,
    Image &Mask);

#endif // search_hpp
