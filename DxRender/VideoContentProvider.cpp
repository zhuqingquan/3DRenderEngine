#include "VideoContentProvider.h"

using namespace zRender;

// unsigned char* RawFrameRefData::getData(int& dataLen, int& pitch, 
// 										int& width, int& height, 
// 										zRender::PIXFormat& pixelFmt, 
// 										RECT& effectReg, int& identify)
// {
// 	if(!isFrameParamValid())
// 		return NULL;
// 	if(m_isUpdatedIdentify<=identify)
// 		return NULL;
// 	dataLen = m_DataLen;
// 	if(dataLen<=0)
// 		return NULL;
// 	pitch = m_Pitch;
// 	width = m_frameWidth;
// 	height = m_frameHeight;
// 	pixelFmt = m_framePixFmt;
// 	
// 	zRender::RECT_f effectiveReg = getEffectiveReg();
// 	effectReg.left = m_frameWidth * effectiveReg.left + 0.5;
// 	effectReg.right = m_frameWidth * effectiveReg.right + 0.5;
// 	effectReg.top = m_frameHeight * effectiveReg.top + 0.5;
// 	effectReg.bottom = m_frameHeight * effectiveReg.bottom + 0.5;
// 	identify = m_isUpdatedIdentify;
// 	return m_pFrameData;
// }

zRender::VideoContentProvider::VideoContentProvider( TextureDataSource* dataSrc )
: m_dataSrc(dataSrc)
, m_rotate(0), m_rotateX(0), m_rotateY(0)
{
	m_vv = new VertexVector(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	zRender::Vertex ver[4];
	ver[0].Pos = XMFLOAT3(0.0, 0.0, 0.0);
	ver[0].Tex = XMFLOAT2(0.0, 1.0);
	ver[1].Pos = XMFLOAT3(1.0, 0.0, 0.0);
	ver[1].Tex = XMFLOAT2(1.0, 1.0);
	ver[2].Pos = XMFLOAT3(0.0, 1.0, 0.0);
	ver[2].Tex = XMFLOAT2(0.0, 0.0);
	ver[3].Pos = XMFLOAT3(1.0, 1.0, 0.0);
	ver[3].Tex = XMFLOAT2(1.0, 0.0);
	UINT index[6] = {0, 2, 3, 0, 3, 1};
	m_vv->addVertexs(ver, 4, index, 6);

	m_vertexIdentify = 1;
}

zRender::VideoContentProvider::~VideoContentProvider()
{
	delete m_vv;
	m_vv = NULL;
	m_vertexIdentify = 0;

	m_dataSrc = NULL;
}

int zRender::VideoContentProvider::getVertexs( VertexVector** vv, int& vvCount, int& identify )
{
	if(vv==NULL || vvCount<1)
		return -1;
	if(m_vertexIdentify<identify)
		return 1;
	*vv = m_vv;
	vvCount = 1;
	identify = m_vertexIdentify;
	return 0;
}

void zRender::VideoContentProvider::increaseAuthorization()
{
	if(m_dataSrc!=NULL)
		m_dataSrc->increaseAuthorization();
}

void zRender::VideoContentProvider::decreaseAuthorization()
{
	if(m_dataSrc!=NULL)
		m_dataSrc->decreaseAuthorization();
}

void* zRender::VideoContentProvider::getShader()
{ return NULL; }

int zRender::VideoContentProvider::setRotation(int rotate)
{
	if(abs(rotate-m_rotate)==180)
	{
		zRender::Vertex* v0 = m_vv->getVertex(0);
		zRender::Vertex* v1 = m_vv->getVertex(1);
		zRender::Vertex* v2 = m_vv->getVertex(2);
		zRender::Vertex* v3 = m_vv->getVertex(3);
		zRender::Vertex ver[4];
		ver[0].Pos = XMFLOAT3(0.0, 0.0, 0.0);
		ver[0].Tex = v3 ? v3->Tex : XMFLOAT2(1.0, 0.0);
		ver[1].Pos = XMFLOAT3(1.0, 0.0, 0.0);
		ver[1].Tex = v2 ? v2->Tex : XMFLOAT2(0.0, 0.0);
		ver[2].Pos = XMFLOAT3(0.0, 1.0, 0.0);
		ver[2].Tex = v1 ? v1->Tex : XMFLOAT2(1.0, 1.0);
		ver[3].Pos = XMFLOAT3(1.0, 1.0, 0.0);
		ver[3].Tex = v0 ? v0->Tex : XMFLOAT2(0.0, 1.0);
		UINT index[6] = {0, 2, 3, 0, 3, 1};
		m_vv->clearVertexs();
		m_vv->addVertexs(ver, 4, index, 6);
		m_vertexIdentify++;
		m_rotate = rotate;
		return 0;
	}
	return -1;
}

int zRender::VideoContentProvider::setRotation_x(int rotate)
{
	if(abs(rotate-m_rotateX)==180)
	{
		zRender::Vertex* v0 = m_vv->getVertex(0);
		zRender::Vertex* v1 = m_vv->getVertex(1);
		zRender::Vertex* v2 = m_vv->getVertex(2);
		zRender::Vertex* v3 = m_vv->getVertex(3);
		zRender::Vertex ver[4];
		ver[0].Pos = XMFLOAT3(0.0, 0.0, 0.0);
		ver[0].Tex = v2 ? v2->Tex : XMFLOAT2(0.0, 0.0);
		ver[1].Pos = XMFLOAT3(1.0, 0.0, 0.0);
		ver[1].Tex = v3 ? v3->Tex : XMFLOAT2(1.0, 0.0);
		ver[2].Pos = XMFLOAT3(0.0, 1.0, 0.0);
		ver[2].Tex = v0 ? v0->Tex : XMFLOAT2(0.0, 1.0);
		ver[3].Pos = XMFLOAT3(1.0, 1.0, 0.0);
		ver[3].Tex = v1 ? v1->Tex : XMFLOAT2(1.0, 1.0);
		UINT index[6] = {0, 2, 3, 0, 3, 1};
		m_vv->clearVertexs();
		m_vv->addVertexs(ver, 4, index, 6);
		m_vertexIdentify++;
		m_rotateX = rotate;
		return 0;
	}

	return -1;
}

int zRender::VideoContentProvider::setRotation_y(int rotate)
{
	if(abs(rotate-m_rotateY)==180)
	{
		zRender::Vertex* v0 = m_vv->getVertex(0);
		zRender::Vertex* v1 = m_vv->getVertex(1);
		zRender::Vertex* v2 = m_vv->getVertex(2);
		zRender::Vertex* v3 = m_vv->getVertex(3);
		zRender::Vertex ver[4];
		ver[0].Pos = XMFLOAT3(0.0, 0.0, 0.0);
		ver[0].Tex = v1 ? v1->Tex : XMFLOAT2(1.0, 1.0);
		ver[1].Pos = XMFLOAT3(1.0, 0.0, 0.0);
		ver[1].Tex = v0 ? v0->Tex : XMFLOAT2(0.0, 1.0);
		ver[2].Pos = XMFLOAT3(0.0, 1.0, 0.0);
		ver[2].Tex = v3 ? v3->Tex : XMFLOAT2(1.0, 0.0);
		ver[3].Pos = XMFLOAT3(1.0, 1.0, 0.0);
		ver[3].Tex = v2 ? v2->Tex : XMFLOAT2(0.0, 0.0);
		UINT index[6] = {0, 2, 3, 0, 3, 1};
		m_vv->clearVertexs();
		m_vv->addVertexs(ver, 4, index, 6);
		m_vertexIdentify++;
		m_rotateY = rotate;
		return 0;
	}
	return -1;
}
