#pragma once
#ifndef _Z_RENDER_ELEM_DSPL_MODEL_H_
#define _Z_RENDER_ELEM_DSPL_MODEL_H_

#include <tchar.h>
#include <D3D11.h>

namespace zRender
{
	class Effect;
	class EffectPass;
	class DxRender;
	class _InputLayoutDesc;
	class InputLayout;

	class ElemDsplModel
	{
	public:
		ElemDsplModel();
		virtual ~ElemDsplModel();

		int init(const TCHAR* shaderFilePathName, DxRender* dxrender);
		void deinit();

		int getEffect(Effect** outEffect) const;
		/**
		 *	@brief		Get the default pass
		 **/
		int getEffectPass(EffectPass** outPass);
		/**
		 *	@brief		Get the pass by the techName and the EffectPass name
		 **/
		int getEffectPass(const char* techName, const char* passName, EffectPass** outPass);
		/**
		 *	@brief		Get the pass by the tech index and the EffectPass index
		 **/
		int getEffectPass(int techIndex, int passIndex, EffectPass** outPass);

		int getInputLayoutDesc(EffectPass* pass, _InputLayoutDesc** layoutDesc);
		int getInputLayout(EffectPass* pass, ID3D11InputLayout** ppLayout);

	private:
		Effect* m_effect;
	};
}

#endif //_Z_RENDER_ELEM_DSPL_MODEL_H_
