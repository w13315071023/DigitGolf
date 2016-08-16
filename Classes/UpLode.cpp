#include "UpLode.h"
#include "PostureAnalysisScene.h"
#include "qrencode.h"
#include <iostream>
#include <fstream>
#include "MyMenu.h"


CCScene* UpLode::getScene()
{
	CCLayer* pLayer = UpLode::create();
	CCScene* pScene = CCScene::create();

	pScene->addChild(pLayer);

	return pScene;
}
UpLode::UpLode(void)
{
	
}


UpLode::~UpLode(void)
{
}
bool UpLode::init()
{
	if (!CCLayerColor::initWithColor(ccc4(0,0,0,128)))
	{
		return false;
	}
	m_QRText = "";
	m_TextNum = 0;
	m_time = 0;
	m_Acting = false;

	m_MsgLabel = CCLabelTTF::create(GBKToUTF8("正在压缩视频。。。").c_str(), "Arial", 72);
	m_MsgLabel->setPosition(ccp(VISIBLEW / 2, VISIBLEH*0.6));
	this->addChild(m_MsgLabel);
	CCMenuItemImage* pItem = CCMenuItemImage::create("SettingUI/qdan1.png", "SettingUI/qdan2.png", this, menu_selector(UpLode::menuCallBack));
	pItem->setPosition(ccp(100, 100));

	MyMenu* pMenu = MyMenu::create(pItem, NULL);
	pMenu->setPosition(ccp(0,0));
	this->addChild(pMenu);

	this->scheduleUpdate();
	return true;
}
void UpLode::menuCallBack(CCObject* obj)
{
	this->unscheduleUpdate();
	CCDirector::sharedDirector()->popScene();
}
void UpLode::update(float dt)
{
	switch (m_time)
	{
		case 0:
			sendRequest();
			m_time = 1;
		break;
		case 1:
			if (m_Acting)
			{
				break;
			}
			uploadVideo("Upload/name1.mp4");
			m_time = 2;
			break;
		case 2:
			if (m_Acting)
			{
				break;
			}
			if (Ext_cameraNum == 2)
			{
				sendRequest();
				m_time = 3;
			}
			else
			{
				m_time = 4;
			}
			break;
		case 3:
			if (m_Acting)
			{
				break;
			}
			if (Ext_cameraNum == 2)
			{
				if (!m_Acting)
				{
					uploadVideo("Upload/name2.mp4");
					m_time = 4;
				}
			}
			break;
		case 4:
			if (m_Acting)
			{
				break;
			}
			QRcodeBMP();
			m_time = 5;
			break;
		case 5:
			if (m_Acting)
			{
				break;
			}
			QRcodeBMPtoPNG();
			m_time = 6;
			break;
		case 6:
			if (m_Acting)
			{
				break;
			}
			m_MsgLabel->setString(GBKToUTF8("上传成功，请扫描下方二维码！").c_str());
			CCSprite* sp = CCSprite::create("Upload/QRcode.png");
			sp->setPosition(ccp(VISIBLEW/2,VISIBLEH*0.3));
			this->addChild(sp);
			m_time = 7;
			break;
	}
}
void UpLode::QRcodeBMP()
{
	m_Acting = true;
	unsigned int	unWidth, x, y, l, n, unWidthAdjusted, unDataBytes;
	unsigned char*	pRGBData, *pSourceData, *pDestData;
	QRcode*			pQRC;
	FILE*			f;

	m_QRText = "http://video.digitgolf.com/?" + m_QRText;
	if (pQRC = QRcode_encodeString(m_QRText.c_str(), 0, QR_ECLEVEL_H, QR_MODE_8, 1))
	{
		unWidth = pQRC->width;
		unWidthAdjusted = unWidth * OUT_FILE_PIXEL_PRESCALER * 3;

		if (unWidthAdjusted % 4)
			unWidthAdjusted = (unWidthAdjusted / 4 + 1) * 4;
		unDataBytes = unWidthAdjusted * unWidth * OUT_FILE_PIXEL_PRESCALER;


		if (!(pRGBData = (unsigned char*)malloc(unDataBytes)))
		{
			printf("Out of memory");
			exit(-1);
		}


		memset(pRGBData, 0xff, unDataBytes);

		BITMAPFILEHEADER kFileHeader;
		kFileHeader.bfType = 0x4d42;  // "BM"
		kFileHeader.bfSize = sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER) +
			unDataBytes;
		kFileHeader.bfReserved1 = 0;
		kFileHeader.bfReserved2 = 0;
		kFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER);

		BITMAPINFOHEADER kInfoHeader;
		kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
		kInfoHeader.biWidth = unWidth * OUT_FILE_PIXEL_PRESCALER;
		kInfoHeader.biHeight = -((int)unWidth * OUT_FILE_PIXEL_PRESCALER);
		kInfoHeader.biPlanes = 1;
		kInfoHeader.biBitCount = 24;
		kInfoHeader.biCompression = BI_RGB;
		kInfoHeader.biSizeImage = 0;
		kInfoHeader.biXPelsPerMeter = 0;
		kInfoHeader.biYPelsPerMeter = 0;
		kInfoHeader.biClrUsed = 0;
		kInfoHeader.biClrImportant = 0;

		pSourceData = pQRC->data;
		for (y = 0; y < unWidth; y++)
		{
			pDestData = pRGBData + unWidthAdjusted * y * OUT_FILE_PIXEL_PRESCALER;
			for (x = 0; x < unWidth; x++)
			{
				if (*pSourceData & 1)
				{
					for (l = 0; l < OUT_FILE_PIXEL_PRESCALER; l++)
					{
						for (n = 0; n < OUT_FILE_PIXEL_PRESCALER; n++)
						{
							*(pDestData + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_B;
							*(pDestData + 1 + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_G;
							*(pDestData + 2 + n * 3 + unWidthAdjusted * l) = PIXEL_COLOR_R;
						}
					}
				}
				pDestData += 3 * OUT_FILE_PIXEL_PRESCALER;
				pSourceData++;
			}
		}
		if (!(fopen_s(&f, "Upload/QRcode.BMP", "wb + ")))
		{
			fwrite(&kFileHeader, sizeof(BITMAPFILEHEADER), 1, f);
			fwrite(&kInfoHeader, sizeof(BITMAPINFOHEADER), 1, f);
			fwrite(pRGBData, sizeof(unsigned char), unDataBytes, f);

			fclose(f);
		}
		else
		{
			printf("Unable to open file");
			exit(-1);
		}
		// Free data

		free(pRGBData);
		QRcode_free(pQRC);
	}
	else
	{
		printf("NULL returned");
		exit(-1);
	}
	m_Acting = false;
}
void UpLode::QRcodeBMPtoPNG()
{
	m_Acting = true;
	int error;
	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION pi;
	string cmdline = "ffmpeg -y -i Upload/QRcode.bmp Upload/QRcode.png";

	TCHAR Name[100];
	MultiByteToWideChar(CP_ACP, 0, cmdline.c_str(), -1, Name, 100);

	error = CreateProcess(TEXT("Upload/ffmpeg.exe"),
		Name,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi);
	error = GetLastError();
	//不使用的句柄最好关掉
	CloseHandle(pi.hThread);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	m_Acting = false;
}
void UpLode::uploadVideo(string name)
{
	m_Acting = true;
	int error;
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		return;
	}
	string cmdline = "Upload/upload.exe "+name + " " + m_Name + ".mp4";

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	TCHAR Name[100];
	MultiByteToWideChar(CP_ACP, 0, cmdline.c_str(), -1, Name, 100);
	if (!CreateProcess(NULL, Name, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		CloseHandle(hWrite);
		CloseHandle(hRead);
		return;
	}
	CloseHandle(hWrite);
	char buffer[4096] = { 0 };
	DWORD bytesRead;
	string strRet;
	while (true)
	{
		if (ReadFile(hRead, buffer, 4095, &bytesRead, NULL) == NULL)
			break;

		strRet.append(buffer, bytesRead);
	}
	CloseHandle(hRead);
	if (strRet != "SUCCESS\r\n")
	{
		m_MsgLabel->setString(GBKToUTF8("视频压缩失败或者视频录制不完整，请返回后重试！").c_str());
		this->unscheduleUpdate();
	}
	CloseHandle(pi.hThread);
	WaitForSingleObject(pi.hProcess, INFINITE);
	CloseHandle(pi.hProcess);
	m_Acting = false;
}
void UpLode::sendRequest()
{
	m_Acting = true;
	CCHttpRequest* request = new CCHttpRequest();
	request->setUrl("http://video.digitgolf.com/getName");
	request->setRequestType(CCHttpRequest::kHttpGet);

	request->setResponseCallback(this, callfuncND_selector(UpLode::onHttpRequestCompleted));
	request->setTag("Get_My_Data");

	CCHttpClient::getInstance()->send(request);
	request->release();
}
void UpLode::onHttpRequestCompleted(cocos2d::CCNode *sender, void *data)
{
	CCHttpResponse *response = (CCHttpResponse*)data;

	if (!response)
	{
		return;
	}

	if (0 != strlen(response->getHttpRequest()->getTag()))
	{
		CCLog("%s completed", response->getHttpRequest()->getTag());
	}

	int statusCode = response->getResponseCode();
	char statusString[64] = {};
	sprintf(statusString, "HTTP Status Code: %d, tag = %s", statusCode, response->getHttpRequest()->getTag());
	CCLog("response code: %d", statusCode);

	if (!response->isSucceed())
	{
		m_MsgLabel->setString(GBKToUTF8("链接失败，请检查网络后重新上传！").c_str());
		this->unscheduleUpdate();
		m_Acting = false;
		return;
	}
	// dump data  
	vector<char> *vc =  response->getResponseData();
	m_Name = string(vc->begin(), vc->end());
	m_FileName = "Upload/" + m_Name + ".mp4";
	m_TextNum++;
	m_MsgLabel->setString(GBKToUTF8("请求成功，正在上传。。。").c_str());
	if (m_TextNum == 2)
	{
		m_QRText += "&name2=" + m_Name;
	}
	else
	{
		m_QRText = "name1=" + m_Name;
	}
	m_Acting = false;
}