#pragma once
#ifndef _SOA_MIRROR_RENDER_BIGVIEWPORT_H_
#define _SOA_MIRROR_RENDER_BIGVIEWPORT_H_

#include "DxRenderCommon.h"
#include <vector>

namespace SOA
{
namespace Mirror
{
namespace Render
{
	class Screen;
	class BigView;
	class BigViewportPartition;

	class BigViewport
	{
	public:
		BigViewport(const zRender::RECT_f& regOfScreen, int zIndex, const Screen& parentSc);
		~BigViewport();

		bool attachView(BigView* bigview);
	private:
		const Screen& m_parentSc;
		std::vector<BigViewportPartition*> m_partitions;
	};
}
}
}

#endif //_SOA_MIRROR_RENDER_BIGVIEWPORT_H_