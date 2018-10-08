#pragma once
#ifndef _Z_RENDER_EFFECT_PASS_H_
#define _Z_RENDER_EFFECT_PASS_H_

#include <string>

class ID3DX11EffectPass;

namespace zRender
{
	class Effect;

	class EffectPass
	{
	public:
		EffectPass(Effect* effect, const char* techName, const char* passName);
		~EffectPass();

		void setIndex(unsigned int techIndex, unsigned int passIndex);
		unsigned int getTechIndex();
		unsigned int getPassIndex();

		const char* getTechName();
		const char* getPassName();

		ID3DX11EffectPass* get();

	private:
		void init(const char* techName, const char* passName);
		void deinit();
	private:
		std::string m_techName;
		std::string m_passName;
		unsigned int m_techIndex;
		unsigned int m_passIndex;
		ID3DX11EffectPass* m_pass;
		Effect* m_effect;
	};
}

#endif //_Z_RENDER_EFFECT_PASS_H_