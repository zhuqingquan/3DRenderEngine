#include "ElementMetaData.h"
#include "Vertex.h"
#include "Effects.h"
#include "ConstDefine.h"
#include "IDisplayContentProvider.h"

using namespace zRender;

zRender::ElementMetaData::ElementMetaData()
	: m_vertexVector(nullptr)
	, m_textureDataSource(nullptr)
	, m_dsplModel(nullptr)
{
	
}

zRender::ElementMetaData::~ElementMetaData()
{
	deinit();
}

int zRender::ElementMetaData::init(DxRender* render, const TCHAR* shaderFileName)
{
	m_vertexVector = new VertexVector(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	zRender::Vertex ver[4];
	ver[0].Pos = XMFLOAT3(0.0, 0.0, 0.0);
	ver[0].Tex = XMFLOAT2(0.0, 1.0);
	ver[1].Pos = XMFLOAT3(1.0, 0.0, 0.0);
	ver[1].Tex = XMFLOAT2(1.0, 1.0);
	ver[2].Pos = XMFLOAT3(0.0, 1.0, 0.0);
	ver[2].Tex = XMFLOAT2(0.0, 0.0);
	ver[3].Pos = XMFLOAT3(1.0, 1.0, 0.0);
	ver[3].Tex = XMFLOAT2(1.0, 0.0);
	UINT index[6] = { 0, 2, 3, 0, 3, 1 };
	m_vertexVector->addVertexs(ver, 4, index, 6);

	m_vertexIdentify = 1;

	createDisplayModel(render, shaderFileName);
	return 0;
}

void zRender::ElementMetaData::deinit()
{
	delete m_dsplModel;
	m_dsplModel = nullptr;
	m_vertexIdentify = 0;
	delete m_vertexVector;
	m_vertexVector = nullptr;
}

bool zRender::ElementMetaData::isValid() const
{
	return m_vertexVector != NULL && m_dsplModel != nullptr;
}

zRender::VertexVector* zRender::ElementMetaData::getVertexData()
{
	return m_vertexVector;
}

bool zRender::ElementMetaData::isTextureNeedUpdate(int& identify)
{
	if(m_textureDataSource==nullptr)
		return false;
	return m_textureDataSource->isUpdated(identify);
}

int zRender::ElementMetaData::update()
{
	return -1;
}

int zRender::ElementMetaData::createDisplayModel(DxRender* render, const TCHAR* shaderFileName)
{
	zRender::ElemDsplModel<zRender::BasicEffect>* pDsplModel = NULL;
	if (0 != zRender::CreateDsplModel<zRender::BasicEffect>(shaderFileName, render, &pDsplModel) || NULL == pDsplModel)
	{
		return DXRENDER_RESULT_FAILED;
	}
	m_dsplModel = pDsplModel;
	return 0;
}