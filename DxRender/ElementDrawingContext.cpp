#include "ElementDrawingContext.h"
#include "DXLogger.h"
#include "IRawFrameTexture.h"
#include "ConstDefine.h"
#include "Vertex.h"
#include "ElementMetaData.h"
#include "DxRenderCommon.h"

using namespace zRender;

#define LOG_TAG _T("ElementDrawingContext")

int ElementDrawingContext::setDisplayRegion(const RECT_f& displayReg, float zIndex)
{
	// 此处假设MetaData中提供的矩形区域是正方形
	// 而如果用户需要将这个区域显示在特定位置或者特定形状，则需要使用此变化矩阵对正方形顶点进行转换
	if (/*displayReg.right<=0 || displayReg.left<0 || displayReg.bottom<=0 || displayReg.top<0
		|| */displayReg.width() <= 0 || displayReg.height() <= 0)
	{
		TCHAR errmsg[1024] = { 0 };
		swprintf_s(errmsg, 1024, L"Error in DisplayElement::setDisplayRegion : param invalid.[L=%f T=%f R=%f B=%f]\n",
			displayReg.left, displayReg.top, displayReg.right, displayReg.bottom);
		log_e(LOG_TAG, errmsg);
		return -2;
	}
	/* 构造转换矩阵
	width, 0,		0, 0
	0,	   height,  0, 0
	0,	   0,		1, 0
	left,  top,	zIndex,	1
	*/
	m_WorldTransformMat._41 = displayReg.left;
	m_WorldTransformMat._42 = -1.0f * (displayReg.height() + displayReg.top);//displayReg.top;
	//m_WorldTransformMat._34 = 0;
	m_WorldTransformMat._11 = displayReg.width();
	m_WorldTransformMat._22 = displayReg.height();
	float fZIndex = zIndex > RANGE_OF_ZINDEX_MAX ? RANGE_OF_ZINDEX_MAX : zIndex;
	fZIndex = zIndex < RANGE_OF_ZINDEX_MIN ? RANGE_OF_ZINDEX_MIN : zIndex;
	m_WorldTransformMat._43 = fZIndex;

	return 0;
}

zRender::ElementDrawingContext::ElementDrawingContext(int width, int height)
	: m_aspectRatio(1.0)
	, m_isEnableTransparent(false), m_alpha(1.0f)
{
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_WorldTransformMat, I);

	//根据窗口的宽高比设置从物体坐标系到world坐标系的基本转换矩阵
	float aspectRatio = width / (float)height;
	//XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_worldBaseTransform, I);
	m_worldBaseTransform._11 = aspectRatio;
	m_aspectRatio = aspectRatio;
}

int zRender::ElementDrawingContext::apply(DxRender* render, IRawFrameTexture* texture, 
	ID3D11Buffer* vtBuf, ID3D11Buffer* indexBuf, ElementMetaData* metadata)
{
	if (nullptr == m_dsplModel)
	{
		return DXRENDER_RESULT_ERROR_IN_MODEL;
	}
	PIXFormat texPixfmt;
	//IRawFrameTexture* texture = this->getTexture();
	if (NULL == texture)
	{
		//不需要渲染Texture时，将PIXFormat设为PIXFMT_UNKNOW，这样就会使用不需要Texture的Shader进行渲染
		texPixfmt = PIXFMT_UNKNOW;
	}
	else
	{
		texPixfmt = texture->getPixelFormat();
	}

	BasicEffect* effect = NULL;
	int ret = m_dsplModel->getEffect(&effect);
	if (NULL == effect)
	{
		return DXRENDER_RESULT_ERROR_IN_MODEL;
	}
	ID3DX11EffectPass* selectedPass = effect->getEffectPass(texPixfmt);
	ID3D11InputLayout* inputLayout = effect->getInputLayout(selectedPass);
	if (inputLayout == NULL)
	{
		TCHAR errmsg[1024] = { 0 };
		swprintf_s(errmsg, 1024, L"Error in DxRender_D3D11::draw : Can not find InputLayout for PixFormat(%d).\n", texPixfmt);
		log_e(LOG_TAG, errmsg);
		return DXRENDER_RESULT_EFFECT_INVALID;
	}
	if (selectedPass == NULL)
	{
		TCHAR errmsg[1024] = { 0 };
		swprintf_s(errmsg, 1024, L"Error in DxRender_D3D11::draw : Can not find Effect pass for PixFormat(%d).\n", texPixfmt);
		log_e(LOG_TAG, errmsg);
		return DXRENDER_RESULT_EFFECT_INVALID;
	}

	//ID3D11Buffer* vtBuf = this->getVertexBuffer();
	//assert(vtBuf);

	//Create and set InputLayout
	VertexVector* vertexVec = metadata->getVertexData();

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11DeviceContext* m_context = (ID3D11DeviceContext*)render->getContext();
	m_context->IASetInputLayout(inputLayout);
	m_context->IASetPrimitiveTopology(vertexVec->getPrimitiveTopology());
	//ID3D11Buffer* vtBuf = displayElem->getVertexBuffer();
	//ID3D11Buffer* indexBuf = this->getIndexBuffer();
	m_context->IASetVertexBuffers(0, 1, &vtBuf, &stride, &offset);
	m_context->IASetIndexBuffer(indexBuf, vertexVec->getIndexBufferFormat(), 0);

	XMMATRIX btf = XMLoadFloat4x4(&m_worldBaseTransform/*m_dxRender->getWorldBaseTransformMatrix()*/);
	XMMATRIX elemWorld = XMLoadFloat4x4(&m_WorldTransformMat/*this->getWorldTransformMatrices()*/);
	elemWorld._41 *= m_aspectRatio;//m_dxRender->getAspectRatio();
	XMMATRIX world = btf * elemWorld;
	XMMATRIX view = XMLoadFloat4x4(&render->getViewTransformMatrix());
	XMMATRIX proj = XMLoadFloat4x4(&render->getProjectionTransformMatrix());
	XMMATRIX worldViewProj = world * view*proj;
	effect->setWorld(world);
	//XMMATRIX worldViewProj = btf*view*proj;
	//m_defaultVideoEffect->SetWorld(btf);
	XMMATRIX worldInvTranspose;
	XMMATRIX A = world;
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR det = XMMatrixDeterminant(A);
	worldInvTranspose = XMMatrixTranspose(XMMatrixInverse(&det, A));
	XMMATRIX I = XMMatrixIdentity();
	XMFLOAT4X4 mTexTransform;
	XMStoreFloat4x4(&mTexTransform, I);
	effect->setWorldInvTranspose(worldInvTranspose);
	effect->setTexTransform(XMLoadFloat4x4(&mTexTransform));
	effect->setWorldViewProj(worldViewProj);
	Material m_material;
	m_material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_material.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	effect->setMaterial(m_material);
	/////////////////////////////set blend state/////////////////////////////////////////
	if (/*alpha!=1.0f*/this->isEnableTransparent())
	{
		float alpha = (float)this->getAlpha();
		effect->setTransparent(alpha);
		//if need transparent effect, setup the transparent blend state
		float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		//fixme create transparent blend state
		//m_context->OMSetBlendState(m_TransparentBS, blendFactors, 0xffffffff);
	}
	else
	{
		//reset to default blend state
		effect->setTransparent(1.0);//reset the Alpha value of Shader
		m_context->OMGetBlendState(NULL, NULL, NULL);
	}

	//IRawFrameTexture* texture = displayElem->getTexture();
	if (texture)
	{
		texture->acquireSync(0, INFINITE);
		/*std::ifstream yuy2FileStream( "D:\\代码黑洞\\datasource\\Frame-720X576.yuy2", std::ios::in | std::ios::binary);
		if(!yuy2FileStream)
		return false;
		int frameWidth = 720;
		int height = 576;
		int yuy2FrameDataLen = frameWidth * height * 2;
		char* frameData = (char*)malloc(yuy2FrameDataLen);
		assert(frameData);
		yuy2FileStream.read(frameData, yuy2FrameDataLen);
		yuy2FileStream.close();
		int tex2dCount = 2;
		ID3D11Texture2D* tex2d[2] = {NULL, NULL};
		texture->getTexture(tex2d, tex2dCount);
		D3D11_MAPPED_SUBRESOURCE mappedRes;
		ZeroMemory(&mappedRes, sizeof(mappedRes));
		D3D11_TEXTURE2D_DESC texDesc;
		tex2d[0]->GetDesc(&texDesc);
		m_context->Map(tex2d[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
		char* pDes = (char*)mappedRes.pData;
		char* pSrc = (char*)frameData;
		int dataPitch = frameWidth*2;
		//assert(subRes.RowPitch==dataPitch);

		for(int i=0; i<height; i++)
		{
		memcpy(pDes, pSrc, dataPitch);
		pDes += mappedRes.RowPitch;
		pSrc += dataPitch;
		}
		m_context->Unmap(tex2d[0], 0);
		free(frameData);
		//texture->update(frameData, yuy2FrameDataLen, 0);
		*/
		int srvCount = 4;
		ID3D11ShaderResourceView* srvList[4] = { NULL, NULL, NULL, NULL };
		texture->getShaderResourceView(srvList, srvCount);
		switch (srvCount)
		{
		case 1:
			effect->setTexture_Y(srvList[0]);
			break;
		case 2:
			effect->setTexture_Y(srvList[0]);
			effect->setTexture_U(srvList[1]);
			break;
		case 3:
			effect->setTexture_Y(srvList[0]);
			effect->setTexture_U(srvList[1]);
			effect->setTexture_V(srvList[2]);
			break;
		default:
			return -6;
			break;
		}
		if (texPixfmt == PIXFMT_YUY2)
		{
			int width = texture->getWidth();
			float dx = 1 / (float)width;
			effect->setDx(dx);
		}
	}

	selectedPass->Apply(0, m_context);
	//VertexVector* vv = this->getVertex();
	size_t indexCount = vertexVec->getIndexCount();//vv ? vv->getIndexCount() : 0;
	m_context->DrawIndexed(indexCount, 0, 0);
	if (texture)
	{
		texture->releaseSync(0);
	}
}

void zRender::ElementDrawingContext::setAlpha(float alpha)
{
	m_alpha = alpha;
	if (m_alpha != 1.0f)
	{
		TCHAR msg[128] = { 0 };
		swprintf(msg, _T("Alpha display enable.[Alpha=%f DisplaElem=%d]\n"), m_alpha, (UINT)this);
		OutputDebugString(msg);
		log_e(LOG_TAG, msg);
	}
}

float zRender::ElementDrawingContext::getAlpha() const
{
	return m_alpha;
}
int ElementDrawingContext::setZIndex(float zIndex)
{
	float fZIndex = zIndex > RANGE_OF_ZINDEX_MAX ? RANGE_OF_ZINDEX_MAX : zIndex;
	fZIndex = zIndex < RANGE_OF_ZINDEX_MIN ? RANGE_OF_ZINDEX_MIN : zIndex;
	m_WorldTransformMat._43 = fZIndex;
	return 0;
}

float zRender::ElementDrawingContext::getZIndex() const
{
	return m_WorldTransformMat._43;
}
