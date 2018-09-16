#include "D3D11TextureRender.h"
#include <Windows.h>
#include <stdio.h>
#include "d3d11StateHelper.h"

using namespace zRender;

#define SAFE_RELEASE(f) if(f) { f->Release(); f=NULL; }
#define NUM_VERTS 4

struct vertex {
	struct {
		float x, y, z, w;
	} pos;
	struct {
		float u, v;
	} tex;
};

static const char vertex_shader_string[] =
"struct VertData \
{ \
	float4 pos : SV_Position; \
	float2 texCoord : TexCoord0; \
}; \
VertData main(VertData input) \
{ \
	VertData output; \
	output.pos = input.pos; \
	output.texCoord = input.texCoord; \
	return output; \
}";

static const char pixel_shader_string[] =
"uniform Texture2D diffuseTexture; \
SamplerState textureSampler \
{ \
	AddressU = Clamp; \
	AddressV = Clamp; \
	Filter   = Linear; \
}; \
struct VertData \
{ \
	float4 pos      : SV_Position; \
	float2 texCoord : TexCoord0; \
}; \
float4 main(VertData input) : SV_Target \
{ \
	return diffuseTexture.Sample(textureSampler, input.texCoord); \
}";

typedef HRESULT(WINAPI *pD3DCompile)
(LPCVOID                         pSrcData,
	SIZE_T                          SrcDataSize,
	LPCSTR                          pFileName,
	CONST D3D_SHADER_MACRO*         pDefines,
	ID3DInclude*                    pInclude,
	LPCSTR                          pEntrypoint,
	LPCSTR                          pTarget,
	UINT                            Flags1,
	UINT                            Flags2,
	ID3DBlob**                      ppCode,
	ID3DBlob**                      ppErrorMsgs);

static pD3DCompile get_compiler(void)
{
	pD3DCompile compile = nullptr;
	char d3dcompiler[40] = {};
	int ver = 49;

	while (ver > 30) {
		sprintf_s(d3dcompiler, 40, "D3DCompiler_%02d.dll", ver);

		HMODULE module = LoadLibraryA(d3dcompiler);
		if (module) {
			compile = (pD3DCompile)GetProcAddress(module,
				"D3DCompile");
			if (compile) {
				break;
			}
		}

		ver--;
	}

	return compile;
}

D3D11TextureRender::D3D11TextureRender(ID3D11Device* device, ID3D11DeviceContext* context)
	: m_device(device), m_context(context)
	, m_initSuccess(false)
{
	createResource();
}

D3D11TextureRender::~D3D11TextureRender()
{
	releaseResource();
}

int D3D11TextureRender::draw(ID3D11Texture2D* texture, ID3D11ShaderResourceView* shaderResView, const RECT& rect, IDXGISwapChain* swapChain)
{
	if (texture == NULL || shaderResView == NULL)
		return -1;
	if (rect.right - rect.left <= 0 || rect.bottom - rect.top <= 0)
		return -2;
	if (m_device == NULL || m_context == NULL)
		return -3;
	if (!m_initSuccess)
		return -4;
	if (NULL == swapChain)
	{
		return -7;
	}
	// 此时为将Texture绘制到Backbuffer中，此时需要为Backbuffer创建RenderTargetView
	if (m_backbufferRTTView == NULL)
	{
		ID3D11Resource* backbuffer = NULL;
		IDXGIResource* dxgiRes = NULL;
		HRESULT hr = swapChain->GetBuffer(0, __uuidof(IUnknown), (void**)&dxgiRes);
		if (FAILED(hr))
			return -8;
		hr = dxgiRes->QueryInterface(__uuidof(ID3D11Resource), (void**)&backbuffer);
		dxgiRes->Release();
		if (FAILED(hr))
		{
			return -9;
		}
		if (!createRenderTargetViewForBackbuffer(backbuffer))
		{
			backbuffer->Release();
			return -5;
		}
		//backbuffer->Release();
	}
	struct d3d11_state oldState = { 0 };
	d3d11_save_state(&oldState, m_context);
	if (!setupPipeline(1, &m_backbufferRTTView, rect, shaderResView))
	{
		d3d11_restore_state(&oldState, m_context);
		return -6;
	}
	m_context->Draw(4, 0);
	d3d11_restore_state(&oldState, m_context);
	return 0;
}

int D3D11TextureRender::draw(ID3D11Texture2D* texture, ID3D11ShaderResourceView* shaderResView, const RECT& rect, UINT uNumRtt, ID3D11RenderTargetView** ppRttArray)
{
	if (texture == NULL || shaderResView == NULL)
		return -1;
	if (rect.right - rect.left <= 0 || rect.bottom - rect.top <= 0)
		return -2;
	if (m_device == NULL || m_context == NULL)
		return -3;
	if (!m_initSuccess)
		return -4;
	if (uNumRtt <= 0 || ppRttArray == NULL)
		return -5;
	struct d3d11_state oldState = { 0 };
	d3d11_save_state(&oldState, m_context);
	if (!setupPipeline(uNumRtt, ppRttArray, rect, shaderResView))
	{
		d3d11_restore_state(&oldState, m_context);
		return -6;
	}
	m_context->Draw(4, 0);
	d3d11_restore_state(&oldState, m_context);
	return -1;
}

bool D3D11TextureRender::setupPipeline(int rttViewCount, ID3D11RenderTargetView** ppRTTView, const RECT& rect, ID3D11ShaderResourceView* shaderResView)
{
	const float factor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	D3D11_VIEWPORT viewport = { 0 };
	UINT stride = sizeof(vertex);
	void *emptyptr = nullptr;
	UINT zero = 0;

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	viewport.TopLeftX = (float)rect.left;
	viewport.TopLeftY = (float)rect.top;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MaxDepth = 1.0f;

	m_context->GSSetShader(nullptr, nullptr, 0);
	m_context->IASetInputLayout(m_vertexLayout);
	m_context->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_context->IASetVertexBuffers(0, 1, &m_vertexBuf, &stride,
		&zero);
	m_context->OMSetBlendState(m_blendState, factor, 0xFFFFFFFF);	//blend_state需要支持透明
	m_context->OMSetDepthStencilState(m_depthState, 0);
	m_context->OMSetRenderTargets(rttViewCount, ppRTTView, /*zstencil_view*/nullptr); 
	m_context->PSSetSamplers(0, 1, &m_samplerState);
	m_context->PSSetShader(m_pixshader, nullptr, 0);
	m_context->RSSetState(m_rasterizerState);
	m_context->RSSetViewports(1, &viewport);
	m_context->SOSetTargets(1, (ID3D11Buffer**)&emptyptr, &zero);
	m_context->VSSetShader(m_vertexShader, nullptr, 0);

	m_context->PSSetShaderResources(0, 1, &shaderResView);					//resourceview则是user content的resourceivew
	return true;
}

bool D3D11TextureRender::createResource()
{
	if (!createPixelShader())
		goto ERR_FAILED;
	if (!createVertexShader())
		goto ERR_FAILED;
	if (!createVertexBuffer())
		goto ERR_FAILED;
	if (!createBlendState())
		goto ERR_FAILED;
	if (!createDepthStencilState())
		goto ERR_FAILED;
	if (!createSamplerState())
		goto ERR_FAILED;
	if (!createRasterizerState())	
		goto ERR_FAILED;
	m_initSuccess = true;
	return true;
ERR_FAILED:
	releaseResource();
	return false;
}
void D3D11TextureRender::releaseResource()
{
	m_initSuccess = false;
	releaseRasterizerState();
	releaseSamplerState();
	releaseDepthStencilState();
	releaseBlendState();
	releaseVertexBuffer();
	releaseVertexShader();
	releasePixelShader();

	releaseBackbufferRenderTargetView();
}

bool D3D11TextureRender::createPixelShader()
{
	ID3D10Blob *blob = NULL;
	HRESULT hr;
	pD3DCompile compile = get_compiler();
	if (!compile) {
		//("hook_dxgi: failed to find d3d compiler library");
		return false;
	}
	hr = compile(pixel_shader_string, sizeof(pixel_shader_string),
		"pixel_shader_string", nullptr, nullptr, "main",
		"ps_4_0", D3D10_SHADER_OPTIMIZATION_LEVEL1, 0, &blob,
		nullptr);
	if (FAILED(hr)) {
		//hlog_hr("hook_dxgi: failed to compile pixel shader", hr);
		return false;
	}

	hr = m_device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr,
		&m_pixshader);
	if (FAILED(hr)) {
		//hlog_hr("d3d11_init_pixel_shader: failed to create shader", hr);
		blob->Release();
		return false;
	}
	blob->Release();
	return true;
}

void D3D11TextureRender::releasePixelShader()
{
	SAFE_RELEASE(m_pixshader);
}

bool D3D11TextureRender::createVertexShader()
{
	ID3D10Blob *blob = NULL;
	HRESULT hr;
	pD3DCompile compile = get_compiler();
	if (!compile) {
		//("hook_dxgi: failed to find d3d compiler library");
		return false;
	}
	hr = compile(vertex_shader_string, sizeof(vertex_shader_string),
		"vertex_shader_string", nullptr, nullptr, "main",
		"vs_4_0", D3D10_SHADER_OPTIMIZATION_LEVEL1, 0, &blob,
		nullptr);
	if (FAILED(hr)) {
		//hlog_hr("hook_dxgi: failed to compile vertex shader", hr);
		return false;
	}
	hr = m_device->CreateVertexShader(blob->GetBufferPointer(), (size_t)blob->GetBufferSize(), nullptr,
		&m_vertexShader);
	if (FAILED(hr)) {
		//hlog_hr("d3d11_user_painter_init_vertex_shader: failed to create shader",
		//	hr);
		blob->Release();
		return false;
	}

	// create vertex input layout
	D3D11_INPUT_ELEMENT_DESC desc[2];
	desc[0].SemanticName = "SV_Position";
	desc[0].SemanticIndex = 0;
	desc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc[0].InputSlot = 0;
	desc[0].AlignedByteOffset = 0;
	desc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[0].InstanceDataStepRate = 0;

	desc[1].SemanticName = "TEXCOORD";
	desc[1].SemanticIndex = 0;
	desc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	desc[1].InputSlot = 0;
	desc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	desc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[1].InstanceDataStepRate = 0;
	hr = m_device->CreateInputLayout(desc, 2, blob->GetBufferPointer(), (size_t)blob->GetBufferSize(),
		&m_vertexLayout);
	if (FAILED(hr)) {
		//hlog_hr("d3d11_init_vertex_shader: failed to create layout",
		//	hr);
		blob->Release();
		return false;
	}
	blob->Release();
	return true;
}

void D3D11TextureRender::releaseVertexShader()
{
	SAFE_RELEASE(m_vertexLayout);
	SAFE_RELEASE(m_vertexShader);
}

bool D3D11TextureRender::createVertexBuffer()
{
	HRESULT hr;
	const vertex verts[NUM_VERTS] = {
		{ { -1.0f,  1.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
		{ { -1.0f, -1.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
		{ { 1.0f,  1.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },
		{ { 1.0f, -1.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } }
	};

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(vertex) * NUM_VERTS;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA srd = {};
	srd.pSysMem = (const void*)verts;

	hr = m_device->CreateBuffer(&desc, &srd, &m_vertexBuf);
	if (FAILED(hr)) {
		//hlog_hr("d3d11_init_vertex_buffer: failed to create vertex "
		//	"buffer", hr);
		return false;
	}

	return true;
}

void D3D11TextureRender::releaseVertexBuffer()
{
	SAFE_RELEASE(m_vertexBuf);
}

bool D3D11TextureRender::createBlendState()
{
	// create BlendState for transparent
	//ID3D11BlendState* transparentBS = nullptr;
	D3D11_BLEND_DESC transparentDesc = { 0 };
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;
	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HRESULT hr = m_device->CreateBlendState(&transparentDesc, &m_blendState);
	if (FAILED(hr))
	{
		//hlog_hr("d3d11_init_user_content_texture: failed to create blend state for transparent.", hr);
		return false;
	}
	return true;
}

void D3D11TextureRender::releaseBlendState()
{
	SAFE_RELEASE(m_blendState);
}

bool D3D11TextureRender::createDepthStencilState()
{
	D3D11_DEPTH_STENCIL_DESC desc = {}; /* defaults all to off */
	HRESULT hr;

	hr = m_device->CreateDepthStencilState(&desc, &m_depthState);
	if (FAILED(hr)) {
		//hlog_hr("d3d11_init_zstencil_state: failed to create "
		//	"zstencil state", hr);
		return false;
	}

	return true;
}

void D3D11TextureRender::releaseDepthStencilState()
{
	SAFE_RELEASE(m_depthState);
}

bool D3D11TextureRender::createSamplerState()
{
	HRESULT hr;

	D3D11_SAMPLER_DESC desc = {};
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	hr = m_device->CreateSamplerState(&desc, &m_samplerState);
	if (FAILED(hr)) {
		//hlog_hr("d3d11_init_sampler_state: failed to create sampler "
		//	"state", hr);
		return false;
	}

	return true;
}

void D3D11TextureRender::releaseSamplerState()
{
	SAFE_RELEASE(m_samplerState);
}

bool D3D11TextureRender::createRasterizerState()
{
	D3D11_RASTERIZER_DESC desc = {};
	HRESULT hr;

	desc.FillMode = D3D11_FILL_SOLID;
	desc.CullMode = D3D11_CULL_NONE;

	hr = m_device->CreateRasterizerState(&desc, &m_rasterizerState);
	if (FAILED(hr)) {
		//hlog_hr("d3d11_init_raster_state: failed to create raster "
		//	"state", hr);
		return false;
	}

	return true;
}

void D3D11TextureRender::releaseRasterizerState()
{
	SAFE_RELEASE(m_rasterizerState);
}

bool D3D11TextureRender::createRenderTargetViewForBackbuffer(ID3D11Resource* backbuffer)
{
	if (nullptr == backbuffer)
		return false;
	HRESULT result = m_device->CreateRenderTargetView(backbuffer, nullptr, &m_backbufferRTTView);
	if (FAILED(result))
	{
		//hlog_hr("d3d11_int_user_content_create_render_target_view: create render target view failed.\n", result);
		return false;
	}
	//hlog("d3d11_int_user_content_create_render_target_view: done\n");
	return true;
}

void D3D11TextureRender::releaseBackbufferRenderTargetView()
{
	SAFE_RELEASE(m_backbufferRTTView);
}