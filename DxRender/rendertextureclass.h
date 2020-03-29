////////////////////////////////////////////////////////////////////////////////
// Filename: rendertextureclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _RENDERTEXTURECLASS_H_
#define _RENDERTEXTURECLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>


////////////////////////////////////////////////////////////////////////////////
// Class name: RenderTextureClass
////////////////////////////////////////////////////////////////////////////////
class RenderTextureClass
{
public:
	RenderTextureClass();
	RenderTextureClass(const RenderTextureClass&);
	~RenderTextureClass();

	bool Initialize(ID3D11Device* device, int width, int height, DXGI_FORMAT fmt);
	void Shutdown();

	void SetRenderTarget(ID3D11DeviceContext*);
	void ClearRenderTarget(ID3D11DeviceContext*, float, float, float, float);
	ID3D11ShaderResourceView* GetShaderResourceView();
	ID3D11Texture2D* getRenderTargetTexture() const { return m_renderTargetTexture; }

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	DXGI_FORMAT GetPixelFormat() const;

	int LockBackbufferHDC(BOOL Discard, HDC* outHDC);
	int unlockBackbufferHDC(HDC hdc);
private:
	ID3D11Texture2D* m_renderTargetTexture;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11ShaderResourceView* m_shaderResourceView;
	ID3D11Texture2D* m_depthBuffer;
	ID3D11DepthStencilView* m_depthView;
	int m_width;
	int m_height;
};

#endif