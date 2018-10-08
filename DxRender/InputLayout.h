#pragma once
#ifndef _Z_RENDER_INPUT_LAYOUT_H_
#define _Z_RENDER_INPUT_LAYOUT_H_

#include <d3d11.h>

namespace zRender
{
	class _InputLayoutDesc
	{
	public:
		_InputLayoutDesc();
		~_InputLayoutDesc();

		D3D11_INPUT_ELEMENT_DESC* getElemDesc();
		unsigned int elemDescCount();

	private:
		D3D11_INPUT_ELEMENT_DESC* m_pElemDescArray;
		unsigned int m_elemCount;
	};
}//namespace zRender

#endif //_Z_RENDER_INPUT_LAYOUT_H_
