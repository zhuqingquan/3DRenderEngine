#pragma once
#ifndef _Z_RENDER_ELEM_DSPL_MODEL_H_
#define _Z_RENDER_ELEM_DSPL_MODEL_H_

#include <tchar.h>
#include <map>
#include <D3D11.h>
#include "DxRenderCommon.h"
#include "DxRender.h"

struct ID3DX11EffectPass;
struct ID3D11InputLayout;

namespace zRender
{
	//class Effect;
	//class BasicEffect;
	class EffectPass;
	class DxRender;
	class _InputLayoutDesc;
	class InputLayout;

	template<typename EffectType>
	class ElemDsplModel
	{
	public:
		ElemDsplModel()
			: m_effect(NULL)
			, m_dxrender(NULL)
		{
		}

		virtual ~ElemDsplModel()
		{
			deinit();
		}

		int init(const TCHAR* shaderFilePathName, DxRender* dxrender)
		{
			if (NULL == initEffect(shaderFilePathName, dxrender))
				return -1;
			return 0;
		}

		void deinit()
		{
			deinitEffect();
		}

		int getEffect(EffectType** outEffect) const
		{
			if (NULL == outEffect)	return -1;
			*outEffect = m_effect;
			return 0;
		}
		/**
		 *	@brief		Get the default pass
		 **/
		//int getEffectPass(EffectPass** outPass);
		/**
		 *	@brief		Get the pass by the techName and the EffectPass name
		 **/
		//int getEffectPass(const char* techName, const char* passName, EffectPass** outPass);
		/**
		 *	@brief		Get the pass by the tech index and the EffectPass index
		 **/
		//int getEffectPass(int techIndex, int passIndex, EffectPass** outPass);

		//int getInputLayoutDesc(EffectPass* pass, _InputLayoutDesc** layoutDesc);
		//int getInputLayout(EffectPass* pass, ID3D11InputLayout** ppLayout);

	protected:
		virtual EffectType* initEffect(const TCHAR* shaderFilePathName, DxRender* dxrender)
		{
			if (m_effect)	return m_effect;
			if (NULL == shaderFilePathName || NULL == dxrender)	return NULL;
			//create and init Effect object
			EffectType* effTmp = new EffectType((ID3D11Device*)dxrender->getDevice(), shaderFilePathName);
			if (NULL == effTmp || !effTmp->isValid())
			{
				return NULL;
			}
			m_dxrender = dxrender;
			m_shaderFile = shaderFilePathName;
			m_effect = effTmp;
			return m_effect;
		}

		virtual void deinitEffect()
		{
			if (m_effect)
			{
				delete m_effect;
				m_effect = NULL;
				m_shaderFile.clear();
			}
		}

/*		virtual bool initEffectPass();
		virtual bool initInputLayout();

	private:
		void createEffectPass(PIXFormat pixfmt);

	private:
		struct EffectPassInputLayoutPair
		{
			ID3DX11EffectPass* effectPass;
			ID3D11InputLayout* inputLayout;

			EffectPassInputLayoutPair()
				: effectPass(NULL), inputLayout(NULL)
			{
			}
		};
		std::map<PIXFormat, EffectPassInputLayoutPair> m_defaultPassAndInputLayout;*/
		EffectType* m_effect;
		DxRender* m_dxrender;
		std::wstring m_shaderFile;
	};

	template<typename EffectType>
	int CreateDsplModel(const TCHAR* shaderFilePathName, DxRender* dxRender, ElemDsplModel<EffectType>** ppModel)
	{
		if (ppModel == NULL)	return -1;
		if (shaderFilePathName == NULL)	return -2;
		if (NULL == dxRender)	return -3;
		ElemDsplModel<EffectType>* model = new ElemDsplModel<EffectType>();
		if (NULL == model)	return -4;
		int ret = model->init(shaderFilePathName, dxRender);
		if (0 != ret)
		{
			return ret;
		}
		*ppModel = model;
		return 0;
	}

	template<typename EffectType>
	void ReleaseDsplModel(ElemDsplModel<EffectType>** ppModel)
	{
		if (ppModel && *ppModel)
		{
			ElemDsplModel<EffectType>* model = *ppModel;
			delete model;
			*ppModel = NULL;
		}
	}
}

#endif //_Z_RENDER_ELEM_DSPL_MODEL_H_
