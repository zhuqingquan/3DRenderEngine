#include "ElemDsplModel.h"
#include "Effects.h"
#include "DxRender.h"
#include <assert.h>
#include <d3dx11effect.h>
#include "Vertex.h"

using namespace zRender;

static ElemDsplModel<BasicEffect> s_basicEffect;		//用于测试模板是否可以编译通过

//ElemDsplModel::ElemDsplModel()
//	: m_effect(NULL)
//	, m_dxrender(NULL)
//{
//
//}
//
//ElemDsplModel::~ElemDsplModel()
//{
//	deinit();
//}
//
//int ElemDsplModel::init(const TCHAR* shaderFilePathName, DxRender* dxrender)
//{
//	if (NULL == initEffect(shaderFilePathName, dxrender))
//		return -1;
//	return 0;
//}
//
//void ElemDsplModel::deinit()
//{
//	deinitEffect();
//}
// 
//Effect* ElemDsplModel::initEffect(const TCHAR* shaderFilePathName, DxRender* dxrender)
//{
//	if (m_effect)	return m_effect;
//	if (NULL == shaderFilePathName || NULL == dxrender)	return NULL;
//	//create and init Effect object
//	BasicEffect* effTmp = new BasicEffect((ID3D11Device*)dxrender->getDevice(), shaderFilePathName);
//	if (NULL == effTmp || !effTmp->isValid())
//	{
//		return NULL;
//	}
//	m_dxrender = dxrender;
//	m_shaderFile = shaderFilePathName;
//	m_effect = effTmp;
//	return m_effect;
//}
/*
void ElemDsplModel::createEffectPass(PIXFormat pixfmt)
{
	ID3DX11EffectPass* pass = m_effect->getEffectPass(pixfmt);
	assert(pass);
	EffectPassInputLayoutPair effInlayoutPair;
	effInlayoutPair.effectPass = pass;
	if (m_defaultPassAndInputLayout.find(pixfmt) != m_defaultPassAndInputLayout.end())
	{
		//TCHAR errmsg[1024] = { 0 };
		//swprintf_s(errmsg, 1024, L"Create Effect pass failed.unsurport Pixfmt.[Fmt=%d]", pixfmt);
		//log_e(LOG_TAG, errmsg);
		return;
	}
	m_defaultPassAndInputLayout[pixfmt] = effInlayoutPair;
}

bool ElemDsplModel::initEffectPass()
{
	createEffectPass(PIXFMT_YUY2);
	createEffectPass(PIXFMT_YUV420P);
	createEffectPass(PIXFMT_YV12);
	createEffectPass(PIXFMT_NV12);
	createEffectPass(PIXFMT_A8R8G8B8);
	createEffectPass(PIXFMT_R8G8B8X8);
	createEffectPass(PIXFMT_R8G8B8);
	createEffectPass(PIXFMT_R8G8B8A8);
	createEffectPass(PIXFMT_B8G8R8A8);
	createEffectPass(PIXFMT_B8G8R8X8);
	createEffectPass(PIXFMT_UNKNOW);
	return true;
}

ID3D11InputLayout* createInputLayout(ID3D11Device* device, ID3DX11EffectPass* effectPass)
{
	ID3D11InputLayout* inputlayout = NULL;
	if (effectPass == NULL || NULL==device)
		return inputlayout;
	D3DX11_PASS_DESC passDesc;
	effectPass->GetDesc(&passDesc);
	if (S_OK != device->CreateInputLayout(InputLayoutDesc::Basic32, 4, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &inputlayout))
	{
#ifdef _DEBUG
		printf("Error in DxRender_D3D11::createInputLayout : CreateInputLayout failed.\n");
#endif
	}
	return inputlayout;
}

bool ElemDsplModel::initInputLayout()
{
	std::map<PIXFormat, EffectPassInputLayoutPair>::iterator iter = m_defaultPassAndInputLayout.begin();
	for (; iter != m_defaultPassAndInputLayout.end(); iter++)
	{
		ID3DX11EffectPass* pass = iter->second.effectPass;
		ID3D11InputLayout* layout = createInputLayout((ID3D11Device*)m_dxrender->getDevice(), pass);
		if (NULL == layout)
			continue;
		iter->second.inputLayout = layout;
	}
	return true;
}
*/

//void ElemDsplModel::deinitEffect()
//{
//	if (m_effect)
//	{
//		delete m_effect;
//		m_effect = NULL;
//		m_shaderFile.clear();
//	}
//}
//
//int ElemDsplModel::getEffect(Effect** outEffect) const
//{
//	if (NULL == outEffect)	return -1;
//	*outEffect = m_effect;
//	return 0;
//}
/*
int ElemDsplModel::getEffectPass(EffectPass** outPass)
{
}

int ElemDsplModel::getEffectPass(const char* techName, const char* passName, EffectPass** outPass)
{

}

int ElemDsplModel::getEffectPass(int techIndex, int passIndex, EffectPass** outPass)
{

}

int ElemDsplModel::getInputLayoutDesc(EffectPass* pass, _InputLayoutDesc** layoutDesc) 
{

}

int ElemDsplModel::getInputLayout(EffectPass* pass, ID3D11InputLayout** ppLayout)
{

}
*/