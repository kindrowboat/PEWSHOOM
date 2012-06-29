#include <iostream>
#include <cv.h>
#include <highgui.h>
#include "stasm.hpp"
#include "stasm_dll.hpp"

int main(){

	cv::VideoCapture cam(0);
	cv::Mat frame;
	Image img;
	char press = ' ';
	cv::Point points[500];
	const cv::Point *pointsPointer = &points[0];

	std::cout << "Hello world.";
	
	if (!cam.isOpened()){
		printf("Could not open camera.");
		return 1;
	}

	while (cam.grab() && press!='q'){
		cam.retrieve(frame);
		cv::imwrite("tempPic.bmp", frame);

		img = Image(frame.cols, frame.rows, false);
		memcpy(img.buf, frame.ptr(), sizeof(img));

		int nlandmarks;
		int landmarks[500];
		const char *image_name = "tempPic.bmp";
		AsmSearchDll(&nlandmarks, landmarks, image_name, (char*)frame.ptr(), frame.cols, frame.rows, 1, NULL, NULL);
		if (nlandmarks == 0) {
			printf("\nError: Cannot locate landmarks in %s\n", image_name);
			//return -1;
		}
		#if 0 // print the landmarks if you want
			printf("landmarks:\n");
			for (int i = 0; i < nlandmarks; i++)
				printf("%3d: %4d %4d\n", i, landmarks[2 * i], landmarks[2 * i + 1]);
		#endif

		for (int x=0; x<nlandmarks; x++){
			points[x].x = landmarks[2*x];
			points[x].y = landmarks[2*x+1];
		}
		//int *p = landmarks;
		//IplImage *img = &(IplImage)frame;
		//cvPolyLine(img, (CvPoint **)&p, &nlandmarks, 1, 1, CV_RGB(255,0,0));
		//cv::polylines(frame, (cv::Point **)&p, &nlandmarks, 1, 1, CV_RGB(255,0,0));
		cv::polylines(frame, &pointsPointer , &nlandmarks, 1, 1, CV_RGB(255,0,0));
		cv::imshow("frame", frame);
		press = cv::waitKey(1);
	}
}