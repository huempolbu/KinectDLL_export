#pragma once
#include <Windows.h>
#include <Kinect.h>

#define DLL_EXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C"
{
#endif
	//kinect vars
	IKinectSensor* m_pKinectSensor = NULL;
	IBodyFrameReader* m_pBodyFrameReader = NULL;
	IColorFrameReader* m_pColorFrameReader = NULL;
	IDepthFrameReader* m_pDepthFrameReader = NULL;

	//function vars
	float* jointPos = nullptr;
	unsigned char* image = nullptr;
	unsigned short* depth = nullptr;

	bool DLL_EXPORT openKinect();
	float DLL_EXPORT *getJoints(float minDistance, float maxDistance);
	unsigned char DLL_EXPORT* getColorImage();
	unsigned short DLL_EXPORT* getDepthImage();
	void DLL_EXPORT closeKinect();

#ifdef __cplusplus
}
#endif