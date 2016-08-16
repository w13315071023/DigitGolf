#pragma once
#include "Tools.h"

#define OUT_FILE_PIXEL_PRESCALER	8											// Prescaler (number of pixels in bmp file for each QRCode pixel, on each dimension)

#define PIXEL_COLOR_R				0											// Color of bmp pixels
#define PIXEL_COLOR_G				0
#define PIXEL_COLOR_B				0xff

class UpLode : public CCLayerColor
{
public:
	UpLode(void);
	~UpLode(void);

	static CCScene* getScene();

	CREATE_FUNC(UpLode);
	bool init();
	void update(float dt);
	void menuCallBack(CCObject* obj);
	void QRcodeBMP();
	void QRcodeBMPtoPNG();
	void uploadVideo(string name);
	void sendRequest();
	void onHttpRequestCompleted(cocos2d::CCNode *sender, void *data);

	int m_TextNum;
	bool m_Acting;
	int m_time;
	string m_FileName;
	string m_Name;
	string m_QRText;
	CCLabelTTF* m_MsgLabel;

};

