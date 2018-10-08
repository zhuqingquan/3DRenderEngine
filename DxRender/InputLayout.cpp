#include "InputLayout.h"
#include <assert.h>

using namespace zRender;

_InputLayoutDesc::_InputLayoutDesc()
	: m_pElemDescArray(NULL)
	, m_elemCount(4)
{
	m_pElemDescArray = (D3D11_INPUT_ELEMENT_DESC*)malloc(m_elemCount * sizeof(D3D11_INPUT_ELEMENT_DESC));
	assert(m_pElemDescArray);
	//fixme init D3D11_INPUT_ELEMENT_DESCs
}

_InputLayoutDesc::~_InputLayoutDesc()
{
	free(m_pElemDescArray);
	m_pElemDescArray = NULL;
	m_elemCount = 0;
}

D3D11_INPUT_ELEMENT_DESC* _InputLayoutDesc::getElemDesc()
{
	return m_pElemDescArray;
}

unsigned int _InputLayoutDesc::elemDescCount()
{
	return m_elemCount;
}