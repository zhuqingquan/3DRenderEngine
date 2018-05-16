//***************************************************************************************
// Effects.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines lightweight effect wrappers to group an effect and its variables.
// Also defines a static Effects class from which we can access all of our effects.
//***************************************************************************************

#pragma once
#ifndef _ZRENDER_EFFECTS_H_
#define _ZRENDER_EFFECTS_H_

#include <string>
#ifdef _WINDOWS

#include <D3D11.h>
#include <d3dx11effect.h>
#include <xnamath.h>
#include "LightHelper.h"
#include "DxRenderCommon.h"

namespace zRender
{

#pragma region Effect
class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	virtual ~Effect();

	bool isValid() const;
private:
	Effect(const Effect& rhs);
	Effect& operator=(const Effect& rhs);

protected:
	ID3DX11Effect* mFX;
};
#pragma endregion

#pragma region BasicEffect
class BasicEffect : public Effect
{
public:
	BasicEffect(ID3D11Device* device, const std::wstring& filename);
	~BasicEffect();

	void setWorldViewProj(CXMMATRIX M)                  { WorldViewProj ? WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)) : S_FALSE; }
	void setWorld(CXMMATRIX M)                          { World ? World->SetMatrix(reinterpret_cast<const float*>(&M)) : S_FALSE; }
	void setWorldInvTranspose(CXMMATRIX M)              { WorldInvTranspose ? WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)) : S_FALSE; }
	void setTexTransform(CXMMATRIX M)                   { TexTransform ? TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)) : S_FALSE; }
	void setEyePosW(const XMFLOAT3& v)                  { EyePosW ? EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)) : S_FALSE; }
	void setDirLights(const DirectionalLight* lights)   { DirLights ? DirLights->SetRawValue(lights, 0, 3*sizeof(DirectionalLight)) : S_FALSE; }
	void setMaterial(const Material& mat)               { Mat ? Mat->SetRawValue(&mat, 0, sizeof(Material)) : S_FALSE; }
	void setTexture_Y(ID3D11ShaderResourceView* tex)   { yTexture->SetResource(tex);/*yTexture ? yTexture->SetResource(tex) : S_FALSE;*/ }
	void setTexture_U(ID3D11ShaderResourceView* tex)   { uTexture->SetResource(tex);/*uTexture ? uTexture->SetResource(tex) : S_FALSE;*/ }
	void setTexture_V(ID3D11ShaderResourceView* tex)   { vTexture->SetResource(tex);/*vTexture ? vTexture->SetResource(tex) : S_FALSE;*/ }
	void setDx(float fDx)							   { dx ? dx->SetRawValue	(&fDx, 0, sizeof(float)) : S_FALSE; }
	void setTransparent(float fTransparent)			   { transparent ? transparent->SetRawValue(&fTransparent, 0, sizeof(float)) : S_FALSE; }

	ID3DX11EffectPass* getEffectPass(PIXFormat pixfmt) const;
	//ID3DX11EffectTechnique* Light1Tech;
	//ID3DX11EffectTechnique* Light2Tech;
	//ID3DX11EffectTechnique* Light3Tech;

	//ID3DX11EffectTechnique* Light0TexTech;
	//ID3DX11EffectTechnique* Light1TexTech;
	//ID3DX11EffectTechnique* Light2TexTech;
	//ID3DX11EffectTechnique* Light3TexTech;
private:
	ID3DX11EffectTechnique* Light1TechNoTex;
	ID3DX11EffectTechnique* Light0TexYUV420Tech;
	ID3DX11EffectTechnique* Light0TexNV12Tech;
	ID3DX11EffectTechnique* Light0TexYUYVTech;
	ID3DX11EffectTechnique* Light0TexARGBTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;
	ID3DX11EffectVariable* dx;
	ID3DX11EffectVariable* transparent;

	ID3DX11EffectShaderResourceVariable* yTexture;
	ID3DX11EffectShaderResourceVariable* uTexture;
	ID3DX11EffectShaderResourceVariable* vTexture;
};
#pragma endregion

}

#endif
#endif // EFFECTS_H