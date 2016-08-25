#include "RecordClass.h"

//pthread_mutex_t RecordClass::m_mutex1 = NULL;
//pthread_mutex_t RecordClass::m_mutex2 = NULL;
RecordClass::RecordClass(void)
{
	m_BufferIndex = 0;
	BufferSize = 0;
}


RecordClass::~RecordClass(void)
{
}
void RecordClass::Destructor()
{
	CameraUnInit(m_hCamera);
	if (!m_Buffer.empty())
	{
		for (size_t i = 0; i < 300; i++)
		{
			CameraAlignFree(m_Buffer[i]->FrameData);
		}
		m_Buffer.clear();
	}
	//if (m_hCamera == 1)
	//{
	//	pthread_mutex_destroy(&m_mutex1);
	//}
	//else
	//{
	//	pthread_mutex_destroy(&m_mutex2);
	//}
	
}
bool RecordClass::init(tSdkCameraDevInfo CameraInfo)
{
	printf("初始化Camera%s\n", CameraInfo.acFriendlyName);
	CameraInit(&CameraInfo, -1, -1, &m_hCamera);
	CameraGetCapability(m_hCamera, &m_sCameraInfo);
	CameraSetAeState(m_hCamera, false);
	CameraPlay(m_hCamera);

	for (size_t i = 0; i < 300; i++)
	{
		VideoRAW* pVideoRGB24 = new VideoRAW();
		pVideoRGB24->FrameData = CameraAlignMalloc(
			m_sCameraInfo.sResolutionRange.iWidthMax*m_sCameraInfo.sResolutionRange.iHeightMax, 16);
		m_Buffer.push_back(pVideoRGB24);
	}

	BufferSize = m_sCameraInfo.sResolutionRange.iWidthMax*m_sCameraInfo.sResolutionRange.iHeightMax * 3;
	
	//if (m_hCamera == 1)
	//{
	//	pthread_mutex_init(&m_mutex1, NULL);
	//}
	//else
	//{
	//	pthread_mutex_init(&m_mutex2, NULL);
	//}

	std::thread myThread(&RecordClass::ThreadCallBack, this);
	SetThreadPriority(myThread.native_handle(), THREAD_PRIORITY_HIGHEST);
	myThread.detach();

	return true;
}
void RecordClass::ThreadCallBack()
{
	BYTE* bydFrameBuffer;
	tSdkFrameHead FrameInfo;
	while (!Ext_IsTurnEnd)
	{
		if (Ext_IsThreadOn)
		{
			if (!CameraGetImageBuffer(m_hCamera, &FrameInfo, &bydFrameBuffer, 20))
			{
				memcpy(m_Buffer[m_BufferIndex]->FrameData, bydFrameBuffer, FrameInfo.uBytes);
				m_Buffer[m_BufferIndex]->FrameHead = FrameInfo;
				m_BufferIndex = (m_BufferIndex + 1) % 300;
				CameraReleaseImageBuffer(m_hCamera, bydFrameBuffer);
			}
		}
	}
	printf("%d相机退出",m_hCamera);
}
VideoRAW* RecordClass::getBufferByIndex(int index)
{
	return m_Buffer[index % 300];
}
int RecordClass::getBuffIndex()
{
	return m_BufferIndex;
}
void RecordClass::setExposureTime(float ExposureTime)
{
	CameraSetExposureTime(m_hCamera, ExposureTime);
	CameraSetOnceWB(m_hCamera);
}
void RecordClass::setGain(int Gain)
{
	CameraSetAnalogGain(m_hCamera,Gain);
}
