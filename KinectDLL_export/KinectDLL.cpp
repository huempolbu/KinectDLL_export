#include "pch.h"
#include "KinectDLL.h"

template <typename Interface>
inline void Release(Interface*& obj)
{
	if (obj == NULL)
		return;
	
	obj->Release();
	obj = NULL;
}

#define CHECK if (FAILED(res)) return false;

bool openKinect()
{
	HRESULT res;
	res = GetDefaultKinectSensor(&m_pKinectSensor);
	CHECK
	res = m_pKinectSensor->Open();
	CHECK

	//color image
	IColorFrameSource* pColorFrameSource = NULL;
	res = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource);
	CHECK
	res = pColorFrameSource->OpenReader(&m_pColorFrameReader);
	CHECK
	Release(pColorFrameSource);

	//depth
	IDepthFrameSource* pDepthFrameSource = NULL;
	res = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
	CHECK
	res = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
	CHECK
	Release(pDepthFrameSource);

	//joints
	IBodyFrameSource* pBodyFrameSource = NULL;
	res = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
	CHECK
	res = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
	CHECK
	Release(pBodyFrameSource);

	if (!m_pKinectSensor)
		return false;
	
	return true;
}

float* getJoints(float minDistance, float maxDistance)
{
	if (jointPos)
	{
		delete[] jointPos;
		jointPos = nullptr;
	}
	IBodyFrame* pBodyFrame = NULL;
	HRESULT res = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
	
	if (FAILED(res))
	{
		Release(pBodyFrame);
		return nullptr;
	}
		

	IBody* ppBodies[BODY_COUNT] = { 0 };
	pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
	for (int i = 0; i < _countof(ppBodies); i++)
	{
		IBody* pBody = ppBodies[i];
		if (!pBody)
			continue;
		BOOLEAN bTracked = false;
		pBody->get_IsTracked(&bTracked);
		if (!bTracked)
			continue;

		Joint joints[JointType_Count];
		pBody->GetJoints(_countof(joints), joints);
		if (joints[21].Position.Z < minDistance || joints[21].Position.Z > maxDistance)
			continue;

		jointPos = new float[3 * _countof(joints)];
		for (int j = 0; j < _countof(joints); j++)
		{
			jointPos[j * 3] = joints[j].Position.X;
			jointPos[j * 3 + 1] = joints[j].Position.Y;
			jointPos[j * 3 + 2] = joints[j].Position.Z;
		}
		break;
	}
	Release(pBodyFrame);
	for (int i = 0; i < _countof(ppBodies); i++)
		Release(ppBodies[i]);

	return jointPos;
}

unsigned char* getColorImage()
{
	if (image)
	{
		delete[] image;
		image = nullptr;
	}
	
	IColorFrame* pColorFrame = NULL;
	int nWidth = 0;
	int nHeight = 0;
	IFrameDescription* pFrameDescription = NULL;
	ColorImageFormat imageFormat = ColorImageFormat_None;

	HRESULT res = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);
	if (FAILED(res))
		return nullptr;

	pColorFrame->get_FrameDescription(&pFrameDescription);
	pFrameDescription->get_Width(&nWidth);
	pFrameDescription->get_Height(&nHeight);
	pColorFrame->get_RawColorImageFormat(&imageFormat);
	int bufSize = nWidth * nHeight * 4;
	image = new unsigned char[bufSize];
	pColorFrame->CopyConvertedFrameDataToArray(bufSize, image, ColorImageFormat_Bgra);

	Release(pFrameDescription);
	Release(pColorFrame);
	return image;
}
unsigned short* getDepthImage()
{
	if (depth)
	{
		delete[] depth;
		depth = nullptr;
	}

	IDepthFrame* pDepthFrame = NULL;
	int nWidth = 0;
	int nHeight = 0;

	IFrameDescription* pFrameDescription = NULL;

	HRESULT res = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);
	if (FAILED(res))
		return nullptr;
	
	pDepthFrame->get_FrameDescription(&pFrameDescription);
	pFrameDescription->get_Width(&nWidth);
	pFrameDescription->get_Height(&nHeight);

	int bufSize = nWidth * nHeight;

	depth = new unsigned short[bufSize];

	pDepthFrame->CopyFrameDataToArray(bufSize, depth);

	Release(pFrameDescription);
	Release(pDepthFrame);

	return depth;
}

void closeKinect()
{
	if (m_pKinectSensor == NULL)
		return;

	Release(m_pDepthFrameReader);
	Release(m_pBodyFrameReader);
	Release(m_pColorFrameReader);
	m_pKinectSensor->Close();
	m_pKinectSensor->Release();
}