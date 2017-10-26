#include "IndependentBigScreenBackground.h"
#include "VideoContentProvider.h"
#include <assert.h>

using namespace SOA::Mirror::Render;
using namespace zRender;

IndependentBigScreenBackground::IndependentBigScreenBackground( const char* imageFileName, 
															   int bigscreenWidth, int bigscreenHeight, 
															   const Size& cellRsolution, zRender::PIXFormat pixfomat )
: BigScreenBackground(imageFileName, bigscreenWidth, bigscreenHeight, cellRsolution, pixfomat)
{

}

IndependentBigScreenBackground::~IndependentBigScreenBackground()
{

}

bool IndependentBigScreenBackground::setBigscreenCellResolution( const Size& resolution )
{
	return false;
}

void IndependentBigScreenBackground::splitBackgroundImageForBigscreenCell()
{
	if(m_isSplited)
		return;
	if(m_decodedData==NULL || m_decodedDataLen==0 || m_regions.size()<=0)
		return;
	for(int i=0; i<m_regions.size(); ++i)
	{
		BigScreenCellPaintRegion& currCellRegion = m_regions[i];
		scaleTheImageFile(currCellRegion.cellWidth, currCellRegion.cellHeight);
		if(m_imageWidth>=currCellRegion.cellWidth)
		{
			currCellRegion.paintRegion.left = 0;
			currCellRegion.paintRegion.right = currCellRegion.cellWidth;
		}
		else
		{
			currCellRegion.paintRegion.left = (currCellRegion.cellWidth - m_imageWidth) / 2;
			currCellRegion.paintRegion.right = currCellRegion.paintRegion.left + m_imageWidth;
		}

		if(m_imageHeight>=currCellRegion.cellHeight)
		{
			currCellRegion.paintRegion.top = 0;
			currCellRegion.paintRegion.bottom = currCellRegion.cellHeight;
		}
		else
		{
			currCellRegion.paintRegion.top = (currCellRegion.cellHeight - m_imageHeight) / 2;
			currCellRegion.paintRegion.bottom = currCellRegion.paintRegion.top + m_imageHeight;
		}

		currCellRegion.pData = m_decodedData;
		currCellRegion.dataLength = m_decodedDataLen;

		float cellPosLeft = (float)(i % m_bigScreenWidth);
		float cellPosTop = (float)(i / m_bigScreenWidth); 
		currCellRegion.displayReg.left = cellPosLeft + currCellRegion.paintRegion.left / (float)currCellRegion.cellWidth;
		currCellRegion.displayReg.right = cellPosLeft + currCellRegion.paintRegion.right / (float)currCellRegion.cellWidth;
		currCellRegion.displayReg.top = cellPosTop + currCellRegion.paintRegion.top / (float)currCellRegion.cellHeight;
		currCellRegion.displayReg.bottom = cellPosTop + currCellRegion.paintRegion.bottom / (float)currCellRegion.cellHeight;

		PictureTextureDataSource* ptds = new RawFrameRefData(m_decodedData,m_decodedDataLen, 
															m_imageWidth, m_imageHeight, FRAMEPITCH(m_imageWidth, m_pixformat), 
															m_pixformat);
		assert(ptds);
		currCellRegion.m_dcproviders = new VideoContentProvider(ptds, RECT_f(0, 1.0, 0.0, 1.0));
	}
	m_isSplited = true;
}
