// $camera.hpp 3.0 milbo$

#if !defined(camera_hpp)
#define camera_hpp

bool fInitCamera(HWND hMainWnd, int iDeviceIndex);
void ShutdownCamera(const char sCameraImg[]);
void WriteCameraImage(const char sCameraImg[]);
int nGetCameraWidth(void);
int nGetCameraHeight(void);
void PrintCapStatus(void);
void IdmCaptureFormat(void);
void IdmCaptureSource(void);

#endif // camera_hpp
