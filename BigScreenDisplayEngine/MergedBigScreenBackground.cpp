#include "MergedBigScreenBackground.h"
#include "VideoContentProvider.h"
#include <assert.h>
#include <Windows.h>

using namespace SOA::Mirror::Render;
using namespace zRender;

MergedBigScreenBackground::MergedBigScreenBackground( const char* imageFileName, 
															   int bigscreenWidth, int bigscreenHeight, 
															   const Size& cellRsolution, zRender::PIXFormat pixfomat )
: BigScreenBackground(imageFileName, bigscreenWidth, bigscreenHeight, cellRsolution, pixfomat)
{

}

MergedBigScreenBackground::~MergedBigScreenBackground()
{
	
}

bool MergedBigScreenBackground::setBigscreenCellResolution( const Size& resolution )
{
	return false;
}

void MergedBigScreenBackground::splitBackgroundImageForBigscreenCell()
{
	if(m_isSplited)
		return;
	if(m_decodedData==NULL || m_decodedDataLen==0 || m_regions.size()<=0)
		return;
	int bigscreenPixelWidth = m_bigScreenWidth * m_regions[0].cellWidth;
	int bigscreenPixelHeight = m_bigScreenHeight * m_regions[0].cellHeight;

	//如果图片大小超过电视墙的分辨率，则先缩小图片，保持纵横比不变
	scaleTheImageFile(bigscreenPixelWidth, bigscreenPixelHeight);

	tagRECT imageRectInBigScreen;
	if(m_imageWidth>=bigscreenPixelWidth)
	{
		imageRectInBigScreen.left = 0;
		imageRectInBigScreen.right = bigscreenPixelWidth;
	}
	else
	{
		imageRectInBigScreen.left = (bigscreenPixelWidth - m_imageWidth) / 2;
		imageRectInBigScreen.right = imageRectInBigScreen.left + m_imageWidth;
	}
	if(m_imageHeight>=bigscreenPixelHeight)
	{
		imageRectInBigScreen.top = 0;
		imageRectInBigScreen.bottom = bigscreenPixelHeight;
	}
	else
	{
		imageRectInBigScreen.top = (bigscreenPixelHeight - m_imageHeight) / 2;
		imageRectInBigScreen.bottom = imageRectInBigScreen.top + m_imageHeight;
	}

	for(int i=0; i<m_regions.size(); ++i)
	{
		BigScreenCellPaintRegion& currCellRegion = m_regions[i];
		int XOfBigscreen = currCellRegion.XOfBigScreen;
		int YOfBigscreen = currCellRegion.YOfBigScreen;
		tagRECT cellRectInBigScreen;
		cellRectInBigScreen.left = XOfBigscreen * currCellRegion.cellWidth;
		cellRectInBigScreen.right = cellRectInBigScreen.left + currCellRegion.cellWidth;
		cellRectInBigScreen.top = YOfBigscreen * currCellRegion.cellHeight;
		cellRectInBigScreen.bottom = cellRectInBigScreen.top + currCellRegion.cellHeight;

		if(cellRectInBigScreen.right <= imageRectInBigScreen.left 
			|| cellRectInBigScreen.left >= imageRectInBigScreen.right
			|| cellRectInBigScreen.top >= imageRectInBigScreen.bottom
			|| cellRectInBigScreen.bottom <= imageRectInBigScreen.top)//背景底图在该窗格上没有任何内容
		{
			currCellRegion.paintRegion.left = currCellRegion.paintRegion.right = currCellRegion.paintRegion.top = currCellRegion.paintRegion.bottom = 0;
			continue;
		}

		//计算背景底图在窗格中的内容覆盖窗格的区域块
		int leftInImageMemory = -1;
		int topInImageMemory = -1;
		int rightInImage = -1;
		int bottomInImage = -1;
		if(imageRectInBigScreen.left >= cellRectInBigScreen.left && imageRectInBigScreen.left<cellRectInBigScreen.right)
		{
			currCellRegion.paintRegion.left = imageRectInBigScreen.left - cellRectInBigScreen.left;
			leftInImageMemory = 0;
		}
		else if(imageRectInBigScreen.left<cellRectInBigScreen.left)
		{
			currCellRegion.paintRegion.left = 0;
			leftInImageMemory = cellRectInBigScreen.left - imageRectInBigScreen.left;
		}

		if(imageRectInBigScreen.right>cellRectInBigScreen.left && imageRectInBigScreen.right<=cellRectInBigScreen.right)
		{
			currCellRegion.paintRegion.right = imageRectInBigScreen.right - cellRectInBigScreen.left;
			rightInImage = m_imageWidth;
		}
		else if(imageRectInBigScreen.right>cellRectInBigScreen.right)
		{
			currCellRegion.paintRegion.right = currCellRegion.cellWidth;
			rightInImage = leftInImageMemory + currCellRegion.paintRegion.getWidth();
		}

		if(imageRectInBigScreen.top >= cellRectInBigScreen.top && imageRectInBigScreen.top<cellRectInBigScreen.bottom)
		{
			currCellRegion.paintRegion.top = imageRectInBigScreen.top - cellRectInBigScreen.top;
			topInImageMemory = 0;
		}
		else if(imageRectInBigScreen.top<cellRectInBigScreen.top)
		{
			currCellRegion.paintRegion.top = 0;
			topInImageMemory = cellRectInBigScreen.top - imageRectInBigScreen.top;
		}

		if(imageRectInBigScreen.bottom>cellRectInBigScreen.top && imageRectInBigScreen.bottom<=cellRectInBigScreen.bottom)
		{
			currCellRegion.paintRegion.bottom = imageRectInBigScreen.bottom - cellRectInBigScreen.top;
			bottomInImage = m_imageHeight;
		}
		else if(imageRectInBigScreen.bottom>cellRectInBigScreen.bottom)
		{
			currCellRegion.paintRegion.bottom = currCellRegion.cellHeight;
			bottomInImage = topInImageMemory + currCellRegion.paintRegion.getHeight();
		}

		//分配对应大小的内存空间
		/*currCellRegion.dataLength = (currCellRegion.paintRegion.right - currCellRegion.paintRegion.left)
									* (currCellRegion.paintRegion.bottom - currCellRegion.paintRegion.top) * SizeOfPixel[m_pixformat];
		if(currCellRegion.dataLength<=0)
			continue;
		currCellRegion.pData = new byte[currCellRegion.dataLength];
		assert(currCellRegion.pData);

		//将显示在该窗格中的图片的内容复制到新申请的内存空间中
		int writepos = 0;
		for(int i=0; i<(currCellRegion.paintRegion.bottom-currCellRegion.paintRegion.top); ++i)
		{
			int startPos = (topInImageMemory+i)*m_imageWidth*SizeOfPixel[m_pixformat] 
							+ leftInImageMemory*SizeOfPixel[m_pixformat];
			int len = (currCellRegion.paintRegion.right - currCellRegion.paintRegion.left) * SizeOfPixel[m_pixformat];
			memcpy(currCellRegion.pData+writepos, m_decodedData+startPos, len);
			writepos += len;
		}*/

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
		RECT_f picEffectiveReg(leftInImageMemory/(float)m_imageWidth, rightInImage/(float)m_imageWidth,
								topInImageMemory/(float)m_imageHeight, bottomInImage/(float)m_imageHeight);
		currCellRegion.m_dcproviders = new VideoContentProvider(ptds, picEffectiveReg);
	}
	//因为分配了新的内存空间，所以可以将解码数据删除
	//delete[] m_decodedData;
	//m_decodedData = NULL;
	//m_decodedDataLen = 0;
	m_isSplited = true;
}

void SOA::Mirror::Render::MergedBigScreenBackground::freeBackgroundContent()
{
	BigScreenBackground::freeBackgroundContent();
	for(int i=0; i<m_regions.size(); ++i)
	{
		if(m_regions[i].pData!=NULL)
		{
			delete[] m_regions[i].pData;
			m_regions[i].pData = NULL;
			m_regions[i].dataLength = 0;
		}
	}
}
