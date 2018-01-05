#include "EffectPass.h"
#include <D3D11.h>
#include <d3dx11effect.h>

using namespace zRender;

EffectPass::EffectPass(Effect* effect, const char* techName, const char* passName)
	: m_pass(NULL), m_effect(effect)
	, m_techIndex(-1), m_passIndex(-1)
{
	if (m_effect == NULL)
		return;
	if (techName == NULL || passName == NULL)
		return;
	init(techName, passName);
}

EffectPass::~EffectPass()
{
	deinit();
}

void EffectPass::init(const char* techName, const char* passName)
{
	//fixme
}

void EffectPass::deinit()
{
	//fixme
}

void EffectPass::setIndex(unsigned int techIndex, unsigned int passIndex)
{
	m_techIndex = techIndex;
	m_passIndex = passIndex;
}

unsigned int EffectPass::getTechIndex()
{
	return m_techIndex;
}

unsigned int EffectPass::getPassIndex()
{
	return m_passIndex;
}

const char* EffectPass::getTechName()
{
	return m_techName.c_str();
}

const char* EffectPass::getPassName()
{
	return m_passName.c_str();
}

ID3DX11EffectPass* EffectPass::get()
{
	return m_pass;
}