//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Effects.h"
#include <fstream>
#include <vector>
#include "DxRenderCommon.h"

using namespace zRender;

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: mFX(0)
{
	//fix me share the memory
	std::ifstream fin(filename.c_str(), std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	if(size<=0)
		return;
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();
	
	if(S_OK!=D3DX11CreateEffectFromMemory(&compiledShader[0], size, 
		0, device, &mFX))
	{
		printf("D3DX11CreateEffectFromMemory failed.\n");
	}
}

Effect::~Effect()
{
	ReleaseCOM(mFX);
}

bool zRender::Effect::isValid() const
{
	return mFX!=NULL;
}

#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
	, Light0TexYUV420Tech(NULL), Light0TexYUYVTech(NULL), Light0TexARGBTech(NULL), Light1TechNoTex(NULL)
	, Light0TexNV12Tech(NULL), WorldViewProj(NULL), World(NULL), WorldInvTranspose(NULL), TexTransform(NULL)
	, EyePosW(NULL), DirLights(NULL), Mat(NULL), dx(NULL)
	, yTexture(NULL), uTexture(NULL), vTexture(NULL)
{
	//Light1Tech    = mFX->GetTechniqueByName("Light1");
	//Light2Tech    = mFX->GetTechniqueByName("Light2");
	//Light3Tech    = mFX->GetTechniqueByName("Light3");

	//Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	//Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	//Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	//Light3TexTech = mFX->GetTechniqueByName("Light3Tex");

	if(NULL==mFX)
		return;
	Light1TechNoTex    = mFX->GetTechniqueByName("Light1");
	Light0TexYUV420Tech = mFX->GetTechniqueByName("Light0Tex_YUV420");
	Light0TexNV12Tech = mFX->GetTechniqueByName("Light0Tex_NV12");
	Light0TexYUYVTech = mFX->GetTechniqueByName("Light0Tex_YUYV");
	Light0TexARGBTech = mFX->GetTechniqueByName("Light0Tex_ARGB");

	WorldViewProj     = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World             = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform      = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();
	DirLights         = mFX->GetVariableByName("gDirLights");
	Mat               = mFX->GetVariableByName("gMaterial");
	dx	              = mFX->GetVariableByName("dx");
	transparent       = mFX->GetVariableByName("transparent");
	yTexture        = mFX->GetVariableByName("yTexture")->AsShaderResource();
	uTexture        = mFX->GetVariableByName("uTexture")->AsShaderResource();
	vTexture        = mFX->GetVariableByName("vTexture")->AsShaderResource();
}

BasicEffect::~BasicEffect()
{
	ReleaseCOM(mFX);
}

ID3DX11EffectPass* BasicEffect::getEffectPass(PIXFormat pixfmt) const
{
	ID3DX11EffectPass* pass = NULL;
	switch (pixfmt)
	{
	case zRender::PIXFMT_YV12:
	case zRender::PIXFMT_YUV420P:
		pass = Light0TexYUV420Tech ? Light0TexYUV420Tech->GetPassByIndex(0) : NULL;
		break;
	case zRender::PIXFMT_YUY2:
		pass = Light0TexYUYVTech ? Light0TexYUYVTech->GetPassByIndex(0) : NULL;
		break;
	case zRender::PIXFMT_A8R8G8B8:
	case zRender::PIXFMT_R8G8B8:
	case zRender::PIXFMT_R8G8B8A8:
	case zRender::PIXFMT_B8G8R8A8:
	case zRender::PIXFMT_B8G8R8X8:
		pass = Light0TexARGBTech ? Light0TexARGBTech->GetPassByIndex(0) : NULL;
		break;
	case zRender::PIXFMT_NV12:
		pass = Light0TexNV12Tech ? Light0TexNV12Tech->GetPassByIndex(0) : NULL;
		break;
	//以下三种形式的像素暂时没有支持的Shader，所以使用默认没有Texture的Shader渲染
	//当外部只需要渲染线条和点，没有Texture时，DxRender_D3D11::draw方法中将会判断这种情况而将PIXFormat设为PIXFMT_UNKNOW
	case zRender::PIXFMT_X8R8G8B8:
	case zRender::PIXFMT_UNKNOW:
		pass = Light1TechNoTex ? Light1TechNoTex->GetPassByIndex(0) : NULL;
		break;
	default:
		break;
	}
	return pass;
}

#pragma endregion