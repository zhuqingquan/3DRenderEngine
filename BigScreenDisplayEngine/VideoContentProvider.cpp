#include "VideoContentProvider.h"

using namespace zRender;
using namespace SOA::Mirror::Render;

unsigned char* RawFrameRefData::getData(int& dataLen, int& yPitch, int& uPitch, int& vPitch,
										int& width, int& height, 
										zRender::PIXFormat& pixelFmt, 
										RECT& effectReg, int& identify)
{
	if(!isFrameParamValid())
		return NULL;
	if(m_isUpdatedIdentify<=identify)
		return NULL;
	dataLen = m_DataLen;
	if(dataLen<=0)
		return NULL;
	yPitch = m_Pitch;
	width = m_frameWidth;
	height = m_frameHeight;
	pixelFmt = m_framePixFmt;
	
	zRender::RECT_f effectiveReg = getEffectiveReg();
	effectReg.left = m_frameWidth * effectiveReg.left + 0.5;
	effectReg.right = m_frameWidth * effectiveReg.right + 0.5;
	effectReg.top = m_frameHeight * effectiveReg.top + 0.5;
	effectReg.bottom = m_frameHeight * effectiveReg.bottom + 0.5;
	identify = m_isUpdatedIdentify;
	return m_pFrameData;
}
