#include "DisplayElement.h"
#include "DxRender_D3D11.h"
#include <assert.h>
#include "DXLogger.h"
#include "SharedFrameTexture.h"
#include "ElemDsplModel.h"

using namespace zRender;
#define LOG_TAG L"DxRender_DisplayElement"

DisplayElement::DisplayElement(DxRender_D3D11* dxRender, ID3D11Device* d3dDevice, ID3D11DeviceContext* context)
	: m_dxRender(dxRender), m_device(d3dDevice), m_context(context)
	, m_IndexBuf(NULL), m_IndexFmt(DXGI_FORMAT_UNKNOWN), m_VertexBuf(NULL)
	, m_texture(NULL), m_isTextureUpdated(false)
	, m_curVerVec(NULL), m_isVertexInfoUpdated(false)
	, m_TexFmt(PIXFMT_UNKNOW), m_TexWidth(0), m_TexHeight(0), m_TexDataSrc(NULL)
	, m_dsplModel(NULL)
	, m_alpha(1.0f), m_isEnableTransparent(false)
{
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&m_WorldTransformMat, I);
}

DisplayElement::~DisplayElement()
{
	releaseRenderResource();
}

int DisplayElement::setDisplayRegion(const RECT_f& displayReg, float zIndex)
{
	if(NULL==m_device || NULL==m_dxRender)
	{
#ifdef _DEBUG
		printf("DisplayElement obj have not be inited yet.\n");
#endif
		return -1;
	}
	if(/*displayReg.right<=0 || displayReg.left<0 || displayReg.bottom<=0 || displayReg.top<0 
		|| */displayReg.width()<=0 || displayReg.height()<=0)	
	{
#ifdef _DEBUG
		printf("Error in DisplayElement::setDisplayRegion : param invalid.[L=%f T=%f R=%f B=%f]\n",
			displayReg.left, displayReg.top, displayReg.right, displayReg.bottom);
#endif
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"Error in DisplayElement::setDisplayRegion : param invalid.[L=%f T=%f R=%f B=%f]\n",
			displayReg.left, displayReg.top, displayReg.right, displayReg.bottom);
		log_e(LOG_TAG, errmsg);
		return -2;
	}
	RECT_f	renderVisibleReg = m_dxRender->getVisibleREgion();
	if(!renderVisibleReg.isIntersect(displayReg))	//该DisplayElement的显示位置与DxRender没有交集
	{
		TCHAR errmsg[512] = {0};
		swprintf_s(errmsg, 512, L"The reg of display element is not intersect with render visible reg.");
		log_e(LOG_TAG, errmsg);
		return -3;
	}
	/* 构造转换矩阵
	width, 0,		0, 0
	0,	   height,  0, 0
	0,	   0,		1, 0
	left,  top,		0,	1
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

int DisplayElement::setDisplayZIndex(float zIndex)
{
	if (NULL == m_device || NULL == m_dxRender)
	{
#ifdef _DEBUG
		printf("Error in DisplayElement::setDisplayZIndex : DisplayElement obj have not be inited yet.\n");
#endif
		return -1;
	}
	float fZIndex = zIndex > RANGE_OF_ZINDEX_MAX ? RANGE_OF_ZINDEX_MAX : zIndex;
	fZIndex = zIndex < RANGE_OF_ZINDEX_MIN ? RANGE_OF_ZINDEX_MIN : zIndex;
	m_WorldTransformMat._43 = fZIndex;
	return 0;
}

float zRender::DisplayElement::getZIndex() const
{
	if (NULL == m_device || NULL == m_dxRender)
	{
#ifdef _DEBUG
		printf("Error in DisplayElement::getZIndex : DisplayElement obj have not be inited yet.\n");
#endif
		return -1.0f;
	}
	return m_WorldTransformMat._43;
}

int DisplayElement::setVertex(VertexVector* vertexInfo)
{
	if(NULL==vertexInfo)
		return -1;
	m_curVerVec = vertexInfo;
	m_IndexFmt = DXGI_FORMAT_R32_UINT;
	m_isVertexInfoUpdated = true;
	return 0;
}

int DisplayElement::setTexture(PIXFormat pixfmt, int width, int height)
{
	if(NULL==m_device || NULL==m_dxRender)
	{
#ifdef _DEBUG
		printf("DisplayElement obj have not be inited yet.\n");
#endif
		return -1;
	}
	if(pixfmt==PIXFMT_UNKNOW || width<=0 || height<=0/* || dataSrc==NULL*/)//fix me
	{
#ifdef _DEBUG
		printf("Error in DisplayElement::setTexture : param invalid.(FMT=%d, W=%d, H=%d)\n",
			pixfmt, width, height);
#endif
		return -2;
	}
	
	if(PixelByteCount[pixfmt]!=PixelByteCount[m_TexFmt] ||
		width!=m_TexWidth || height!=m_TexHeight)
		m_isTextureUpdated = true;
	m_TexFmt = pixfmt;
	m_TexHeight = height;
	m_TexWidth = width;
	return 0;
}

int DisplayElement::setTextureDataSource(TextureDataSource* dataSrc, const RECT_f& textureReg)
{
	if(dataSrc==NULL)//如果dataSrc为NULL，则忽略对textureReg参数的检查
	{
		m_TexDataSrc = dataSrc;
		return 0;
	}
	if(textureReg.left<0 || textureReg.left>=1 || textureReg.top<0 || textureReg.top>=1
		|| textureReg.width() <= 0 || textureReg.height()<=0 || textureReg.right>1 ||textureReg.bottom>1)
	{
#ifdef _DEBUG
		printf("Error in DisplayElement::setTextureDataSource : param [textureReg] is invalid.(L=%f, R=%f, T=%f, B=%f)\n",
			textureReg.left, textureReg.right, textureReg.top, textureReg.bottom);
#endif
		return -1;
	}
	m_TexDataSrc = dataSrc;
	m_TexEffectiveReg = textureReg;
	return 0;
}

int DisplayElement::createRenderResource()
{
	if(NULL==m_device || NULL==m_dxRender)
	{
#ifdef _DEBUG
		printf("DisplayElement obj have not be inited yet.\n");
#endif
		return -1;
	}
	if(m_isTextureUpdated)		//创建Texture
	{
		int ret = releaseTexTureResource();
		if(0!=ret)
			return -2;
		ret = createTextureResource();
		if(0!=ret)
			return -3;
		m_isTextureUpdated = false;
	}
	if(m_isVertexInfoUpdated)	//创建Vertex、Index Buffer
	{
		int ret = releaseIndexBuffer();
		if(0!=ret)
			return -4;
		ret = createIndexBuffer();
		if(0!=ret)
			return -5;
		ret = releaseVertexBuffer();
		if(0!=ret)
			return -6;
		ret = createVertexBuffer();
		if(0!=ret)
			return -7;
		m_isVertexInfoUpdated = false;
	}
	return 0;
}

int DisplayElement::releaseRenderResource()
{
	releaseVertexBuffer();
	releaseIndexBuffer();
	releaseTexTureResource();
	return 0;
}

int DisplayElement::createTextureResource()
{
	if(m_texture!=NULL)	return -1;
	IRawFrameTexture* texTmp = m_dxRender->createTexture(m_TexFmt, m_TexWidth, m_TexHeight);
	if(texTmp==NULL)
		return -2;
	m_texture = texTmp;
	return 0;
}

int DisplayElement::releaseTexTureResource()
{
	if(m_texture)
	{
		int ret = m_texture->destroy();
		if(0!=ret)
			return ret;
		delete m_texture;
		m_texture = NULL;
	}
	return 0;
}

int DisplayElement::createVertexBuffer()
{
	assert(m_curVerVec);
	if(m_VertexBuf)	return -1;
	int initDataLen = 0;
	const unsigned char* initData = m_curVerVec->getVertexData(initDataLen);
	if(NULL==initData || initDataLen<=0)
		return -2;
	ID3D11Buffer* vb = m_dxRender->createVertexBuffer(initDataLen, initData, initDataLen);
	if(vb==NULL)
		return -3;
	m_VertexBuf = vb;
	return 0;
}

int DisplayElement::releaseVertexBuffer()
{
	return m_dxRender->releaseBuffer(&m_VertexBuf);
}

int DisplayElement::createIndexBuffer()
{
	assert(m_curVerVec);
	if(m_IndexBuf)	return -1;
	int initDataLen = 0;
	const unsigned char* initData = m_curVerVec->getIndexData(initDataLen);
	if(NULL==initData || initDataLen<=0)
		return -2;
	ID3D11Buffer* ib = m_dxRender->createIndexBuffer(initDataLen, initData, initDataLen);
	if(ib==NULL)
		return -3;
	m_IndexBuf = ib;
	return 0;
}

int DisplayElement::releaseIndexBuffer()
{
	return m_dxRender->releaseBuffer(&m_IndexBuf);
}

bool DisplayElement::isValid() const
{
	if(m_VertexBuf==NULL || m_IndexBuf==NULL)
		return false;
	return true;
}

int DisplayElement::updateTexture(int& identify)
{
	if(m_context==NULL)
	{
#ifdef _DEBUG
		printf("Error in DisplayElement::updateTexture : m_context is NULL.\n");
#endif
		return -1003;
	}
	if(!this->isValid())
	{
#ifdef _DEBUG
		printf("DisplayElement Not init.Resource have not been created.\n");
#endif
		return -1001;
	}
	if(m_texture==NULL || m_TexDataSrc==NULL)
	{
		return -1002;
	}
	int tex2dCount = 4;
	ID3D11Texture2D* tex2d[4] = {NULL, NULL, NULL, NULL};
	m_texture->getTexture(tex2d, tex2dCount);
	if(tex2dCount<=0)
		return -1004;

 	RECT effectReg;
 	//zRender::SharedTexture* shTex = m_TexDataSrc->getSharedTexture(effectReg, identify);
	zRender::IRawFrameTexture* tex = m_TexDataSrc->getTexture();
 	if(NULL!=tex)
 	{
	int dataLen, width, height, yPitch, uPitch, vPitch;
	PIXFormat pixfmt;
	unsigned char* pData = m_TexDataSrc->getData(dataLen, yPitch, uPitch, vPitch, width, height, pixfmt, effectReg, identify);
	int effectRegWidth = effectReg.right-effectReg.left;
	int effectRegHeight = effectReg.bottom-effectReg.top;
	if(pData==NULL || dataLen<=0 || width<=0 || height<=0 
		|| yPitch<=0 || pixfmt!=m_texture->getPixelFormat() || effectReg.left<0 || effectReg.top<0
		|| effectRegWidth<=0 || effectRegHeight<=0 || effectReg.bottom>height || effectReg.right>width)
		return -1005;

	RECT copyedReg;
	copyedReg.left = (LONG)(effectReg.left + (effectRegWidth * m_TexEffectiveReg.left + 0.5));
	copyedReg.right = (LONG)(effectReg.left + (effectRegWidth * m_TexEffectiveReg.right + 0.5));
	copyedReg.top = (LONG)(effectReg.top + (effectRegHeight * m_TexEffectiveReg.top + 0.5));
	copyedReg.bottom = (LONG)(effectReg.top + (effectRegHeight * m_TexEffectiveReg.bottom + 0.5));
	return m_TexDataSrc->copyDataToTexture(RECT_f(0, 1, 0, 1), pData, yPitch, height, identify);
	return m_texture->update(pData, dataLen, yPitch, uPitch, vPitch, width, height, copyedReg, m_context);
	return -1005;
	}
 	//else
 	//{
 	//	return m_texture->update(shTex, effectReg, m_context);
 	//}
	////////////////////////////////////////////////////////////////////////////////
// 	D3D11_MAPPED_SUBRESOURCE mappedRes;
// 	ZeroMemory(&mappedRes, sizeof(mappedRes));
// 	D3D11_TEXTURE2D_DESC texDesc;
// 	tex2d[0]->GetDesc(&texDesc);
// 	m_context->Map(tex2d[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
// 	unsigned char* pDes = (unsigned char*)mappedRes.pData;
// 	m_TexDataSrc->copyDataToTexture(m_TexEffectiveReg, pDes, mappedRes.RowPitch, m_TexHeight, identify);
// 	m_context->Unmap(tex2d[0], 0);
// 	return 0;
}

void zRender::DisplayElement::setAlpha( float alpha )
{
	m_alpha = alpha;
}

float zRender::DisplayElement::getAlpha() const
{
#ifdef _DEBUG
	if(m_alpha!=1.0f)
	{
		char msg[128] = {0};
		sprintf(msg, ("Alpha display enable.[Alpha=%f DisplaElem=%d]\n"), m_alpha, (UINT)this);
		OutputDebugStringA(msg);
	}
#endif
	return m_alpha;
}

void zRender::DisplayElement::enableTransparent( bool enable )
{
	m_isEnableTransparent = enable;
}

int zRender::DisplayElement::openSharedTexture(IRawFrameTexture * sharedTexture)
{
	if (sharedTexture == NULL)
		return -1;
	if (m_texture)
	{
		//releaseTexTureResource();
		return 0;
	}
	m_texture = m_dxRender->openSharedTexture(sharedTexture);
	if (NULL == m_texture)
		return -2;
	return 0;
}

int zRender::DisplayElement::draw()
{
	//如果没有显示模型，则该Element无法完成显示，因为根本不知道该如何显示
	if (m_dsplModel == NULL)
		return -1;
	if(0!=this->createRenderResource() || !this->isValid() )
	{
		#ifdef _DEBUG
		printf("Error in DxRender_D3D11::draw : DisplayElement is invalid.\n");
		#endif
		TCHAR errmsg[512] = {0};
		swprintf_s(errmsg, 512, L"Error in DxRender_D3D11::draw : DisplayElement is invalid.");
		log_e(LOG_TAG, errmsg);
		return -4;
	}
	PIXFormat texPixfmt;
	IRawFrameTexture* texture = this->getTexture();
	if(NULL==texture)
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
		//fixme log
		return -2;
	}
	ID3DX11EffectPass* selectedPass = effect->getEffectPass(texPixfmt);
	ID3D11InputLayout* inputLayout = effect->getInputLayout(selectedPass);
	//if(displayElem->getShader()==NULL)
	//{
	//assert(m_defaultVideoEffect);
	//selectedPass = findEffectPass(texPixfmt);
	//inputLayout = findInputLayout(texPixfmt);
	//}
	if(inputLayout==NULL)
	{
		#ifdef _DEBUG
			printf("Error in DxRender_D3D11::draw : Can not find InputLayout for PixFormat(%d).\n", texPixfmt);
		#endif
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"Error in DxRender_D3D11::draw : Can not find InputLayout for PixFormat(%d).\n", texPixfmt);
		log_e(LOG_TAG, errmsg);
		return -3;
	}
	if(selectedPass==NULL)
	{
		#ifdef _DEBUG
			printf("Error in DxRender_D3D11::draw : Can not find Effect pass for PixFormat(%d).\n", texPixfmt);
		#endif
		TCHAR errmsg[1024] = {0};
		swprintf_s(errmsg, 1024, L"Error in DxRender_D3D11::draw : Can not find Effect pass for PixFormat(%d).\n", texPixfmt);
		log_e(LOG_TAG, errmsg);
		return -5;
	}
	
	ID3D11Buffer* vtBuf = this->getVertexBuffer();
	assert(vtBuf);

	//Create and set InputLayout

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_context->IASetInputLayout(inputLayout);
	m_context->IASetPrimitiveTopology(this->getVertex()->getPrimitiveTopology());
	//ID3D11Buffer* vtBuf = displayElem->getVertexBuffer();
	ID3D11Buffer* indexBuf = this->getIndexBuffer();
	m_context->IASetVertexBuffers(0, 1, &vtBuf, &stride, &offset);
	m_context->IASetIndexBuffer(indexBuf, this->getIndexBufferFormat(), 0);

	XMMATRIX btf = XMLoadFloat4x4(&m_dxRender->getWorldBaseTransformMatrix());
	XMMATRIX elemWorld = XMLoadFloat4x4(&this->getWorldTransformMatrices());
	//elemWorld._11 *= m_aspectRatio;
	//elemWorld._21 *= m_aspectRatio;
	//elemWorld._31 *= m_aspectRatio;
	elemWorld._41 *= m_dxRender->getAspectRatio();
	XMMATRIX world = btf * elemWorld;
	XMMATRIX view  = XMLoadFloat4x4(&m_dxRender->getViewTransformMatrix());
	XMMATRIX proj  = XMLoadFloat4x4(&m_dxRender->getProjectionTransformMatrix());
	XMMATRIX worldViewProj = world*view*proj;
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
	if(/*alpha!=1.0f*/this->isEnableTransparent())
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
	VertexVector* vv = this->getVertex();
	size_t indexCount = vv ? vv->getIndexCount() : 0;
	m_context->DrawIndexed(indexCount, 0, 0);
	if (texture)
	{
		texture->releaseSync(0);
	}
	return 0;
}