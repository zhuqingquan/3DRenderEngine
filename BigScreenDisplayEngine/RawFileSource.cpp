#include "RawFileSource.h"
#include "BigView.h"

using namespace zRender;
using namespace SOA::Mirror::Render;

zRender::RawFileSource::RawFileSource(DxRender * dxrender)
	: m_textureSource(new SharedTextureSource(dxrender))
	, m_width(0), m_height(0), m_pixfmt(zRender::PIXFMT_UNKNOW)
	, m_fileStream(NULL), m_thUpdate(NULL), m_thUpdateRunning(false)
	, m_fps(0)
{
}

zRender::RawFileSource::~RawFileSource()
{
	stop();
	close();
	if (m_textureSource)
	{
		delete m_textureSource;
		m_textureSource = NULL;
	}
}

bool zRender::RawFileSource::open(const TCHAR * filePathName, zRender::PIXFormat pixfmt, int width, int height)
{
	if (NULL == filePathName)	return false;
	if (NULL != m_fileStream)	return false;

	std::ifstream* filestream = new std::ifstream(filePathName, std::ios::binary | std::ios::in);
	if (NULL == filestream || !(*filestream))
	{
		delete filestream;
		return false;
	}
	m_fileStream = filestream;
	m_width = width;
	m_height = height;
	m_pixfmt = pixfmt;
	m_textureSource->createTexture(m_pixfmt, width, height);
	return true;
}

void zRender::RawFileSource::close()
{
	if (m_fileStream)
	{
		m_fileStream->close();
		delete m_fileStream;
		m_fileStream = NULL;
		m_width = 0;
		m_height = 0;
		m_pixfmt = PIXFMT_UNKNOW;
	}
}

DWORD WINAPI doReadFileAndUpdateTexture(LPVOID param)
{
	RawFileSource* pThis = reinterpret_cast<RawFileSource*>(param);
	if (pThis) pThis->doReadFileAndUpdateToTexture();
	return 0;
}
bool zRender::RawFileSource::start(int fps)
{
	if (NULL != m_thUpdate) return false;
	if (fps <= 0)	return false;
	m_thUpdateRunning = true;
	HANDLE thUpdate = CreateThread(NULL, 0, doReadFileAndUpdateTexture, this, 0, 0);
	if (NULL == thUpdate)	return false;
	m_thUpdate = thUpdate;
	m_fps = fps;
	return true;
}

void zRender::RawFileSource::stop()
{
	if (NULL == m_thUpdate) return;
	m_thUpdateRunning = false;
	WaitForSingleObject(m_thUpdate, INFINITE);
	CloseHandle(m_thUpdate);
	m_thUpdate = NULL;
}

void zRender::RawFileSource::doReadFileAndUpdateToTexture()
{
	if (NULL == m_fileStream || m_fps<=0)	return;
	if (m_fileStream->eof())	return;
	int duration = 1000 / m_fps;
	if (duration <= 0)
		return;
	size_t frameLen = 0;
	int pitch = 0;
	int idt = 0;
	switch (m_pixfmt)
	{
	case zRender::PIXFMT_UNKNOW:
		return;
	case zRender::PIXFMT_YUV420P:
		pitch = m_width;
		frameLen = pitch * m_height + (pitch / 2) * (m_height >> 1) * 2;
		break;
	case zRender::PIXFMT_YUY2:
		pitch = m_width * 2;
		frameLen = pitch * m_height;
		break;
	case zRender::PIXFMT_R8G8B8:
		pitch = m_width * 3;
		frameLen = pitch * m_height;
		break;
	case zRender::PIXFMT_NV12:
		pitch = m_width;
		frameLen = pitch * m_height + pitch * (m_height >> 1);
		break;
	case zRender::PIXFMT_YV12:
		pitch = m_width;
		frameLen = pitch * m_height + pitch * (m_height >> 1);
		break;
	case zRender::PIXFMT_A8R8G8B8:
	case zRender::PIXFMT_X8R8G8B8:
	case zRender::PIXFMT_R8G8B8A8:
	case zRender::PIXFMT_B8G8R8A8:
	case zRender::PIXFMT_B8G8R8X8:
		pitch = m_width * 4;
		frameLen = pitch * m_height;
		break;
	default:
		return;
	}
	unsigned char* pOneFrame = (unsigned char*)malloc(frameLen);
	while (m_thUpdateRunning)
	{
		if (!(*m_fileStream))
		{
			m_fileStream->clear();
			m_fileStream->seekg(0, std::ios::beg);
		}
		m_fileStream->read((char*)pOneFrame, frameLen);
		//m_textureSource->copyDataToTexture(RECT_f(0, 1, 0, 1), pOneFrame, pitch, m_height, ++idt);	
		m_textureSource->cacheData(RECT_f(0, 1, 0, 1), pOneFrame, pitch, m_width, m_height);
		Sleep(duration);
	}
	free(pOneFrame);
}

SOA::Mirror::Render::BigView * zRender::RawFileSource::createSourceView()
{
	SOA::Mirror::Render::BigView* srcview = new SOA::Mirror::Render::BigView(zRender::RECT_f(0, 1, 0, 1));
	srcview->attachTextureSource(m_textureSource);
	return srcview;
}

void zRender::RawFileSource::releaseSourceView(SOA::Mirror::Render::BigView ** srcView)
{
	if (srcView == NULL)	return;
	SOA::Mirror::Render::BigView* psrcview = *srcView;
	delete psrcview;
	*srcView = NULL;
}
