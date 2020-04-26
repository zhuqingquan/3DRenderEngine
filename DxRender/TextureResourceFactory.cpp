#include "TextureResourceFactory.h"
#include "inc/TextureResource.h"
#include "PackedYUVTextureResource.h"

using namespace zRender;

ITextureResource* zRender::TextureResourceFactory::create(PIXFormat pixfmt)
{
	switch (pixfmt)
	{
	case PIXFMT_A8R8G8B8:
	case PIXFMT_B8G8R8A8:
	case PIXFMT_R8G8B8X8:
	case PIXFMT_R8G8B8A8:
	case PIXFMT_B8G8R8X8:
	case PIXFMT_R8G8B8:
		return new TextureResource();
		break;
	case PIXFMT_NV12:
		break;
	case PIXFMT_YV12:
		break;
	case PIXFMT_YUV420P:
		break;
	case PIXFMT_YUY2:
		return new PackedYUVTextureResource();
		break;
	default:
		return nullptr;
	}
	return nullptr;
}
