/**
 *	@author		zhuqingquan
 *	@date		2014-10-17
 *	@name		DxRender_D3D11.h
 *	@brief		The Display engine impleted in DirextX11 3D
 */

#pragma once
#ifndef _ZRENDER_DXRENDER_D3D11_H_
#define _ZRENDER_DXRENDER_D3D11_H_

//because this display engine is impletemented by directx11 3D
//so it can only compile in windows
#ifdef _WINDOWS

#include <Windows.h>
#include <d3d11.h>
#include <xnamath.h>
#include <map>
#include "DxRenderCommon.h"
#include "DisplayElement.h"
#include "Effects.h"
#include "BackgroundDisplayComponent.h"

namespace zRender
{
	class SharedTexture;

	/**
	 *	@name	DxRender_D3D11
	 *	@brief	使用DirectX11 3D技术实现的显示引擎
	 */
	class DxRender_D3D11
	{
	public:
		/**
		 *	@name	DxRender_D3D11
		 *	@brief	构造方法，对象创建还需调用init初始化资源
		 */
		DxRender_D3D11();

		/**
		 *	@name	~DxRender_D3D11
		 *	@brief	析构方法，释放所有创建的资源
		 */
		~DxRender_D3D11();

		/**
		 *	@name		setVisibleRegion
		 *	@brief		设置该Device显示坐标系中的区域，只有在这个区域内的事物才能被显示
		 *	@param[in]	RECT_f& visibleReg 坐标系中的区域，不考虑z坐标
		 *	@return		int 0--成功  非0--失败
		 */
		int setVisibleRegion(const RECT_f& visibleReg);

		/**
		 *	@name	getVisibleREgion
		 *	@brief	获取该Device显示坐标系中的区域
		 **/
		RECT_f getVisibleREgion() const;

		/**
		 *	@name		init
		 *	@brief		初始化显示引擎，创建显示所需的资源
		 *	@param[in]	HWND hWnd 用于显示内容的Windows窗口句柄
		 *	@param[in]	const wchar_t* effectFileName 显示所需的Shader文件，effect文件的路径名称
		 *				可以是相对路径或者绝对路径
		 *	@param[in]	isEnable4XMSAA 是否支持MSAA，默认为false
		 *				如果该参数为true，则isSDICompatible不能为true
		 *	@param[in]	isSDICompatible 是否支持获取HDC句柄用于GDI渲染
		 *				如果该参数为false，则lockBackbufferHDC接口调用失败
		 *				如果该参数为true，则isEnable4XMSAA不能为true
		 *	@return		int 0--成功  非0--失败
		 */
		int init(HWND hWnd, const wchar_t* effectFileName, bool isEnable4XMSAA = false, bool isSDICompatible = false);

		/**
		 *	@name		init
		 *	@brief		该方法将使用hmonitor桌面连接的Adapter上创建Device
		 *				该方法只创建Device，而不创建其他present所需的资源，这样可以用于createSharedTexture接口的调用
		 *	@param[in]	HMONITOR hmonitor 系统桌面的句柄
		 *	@return		int 0--成功  非0--失败
		 */
		int init(HMONITOR hmonitor);

		/**
		 *	@name		deinit
		 *	@brief		释放显示引擎中创建的资源
		 */
		void deinit();

		/**
		 *	@name		createDisplayElement
		 *	@brief		创建DisplayElement对象，代表一个显示的内容
		 *				创建时，与DisplayElement对应的资源还未创建，资源在调用DisplayElement的接口时创建
		 *				DxRender对象正确初始化之后才能调用创建接口创建各种资源对象
		 *	@param[in]	const RECT_f& displayReg 该显示的内容在坐标系中的位置
		 *	@param[in]	int zIndex 当前显示原始所在顶点的Z坐标偏移
		 *	@return		DisplayElement* 对象指针，失败时为NULL，但参数displayReg表示的位置不在本Render的可视区域内时创建失败
		 */
		DisplayElement* createDisplayElement(const RECT_f& displayReg, int zIndex);
		
		/**
		 *	@name		releaseDisplayElement
		 *	@brief		释放DisplayElement对象
		 *				释放DisplayElement对象中的资源，并删除对象
		 *	@param[in]	DisplayElement** displayElement 指向DisplayElement对象指针的指针，DisplayElement对象释放成功的则该指针被置NULL
		 *	@return		int 0--成功   <0--失败
		 */
		int releaseDisplayElement(DisplayElement** displayElement);

		int createSharedTexture(SharedTexture** ppSharedTex, PIXFormat pixfmt);

		int releaseSharedTexture(SharedTexture** ppSharedTex);

		/**
		 *	@name		createTexture
		 *	@brief		创建Texture对象
		 *				Texture对象可能是一张完整的图片或者图片的一部分，Texture会申请ID3D11Texture2D资源
		 *				DxRender对象正确初始化之后才能调用创建接口创建各种资源对象
		 *	@param[in]	YUVFormat_Packed pixFmt Texture中的像素格式
		 *	@param[in]	int width Texture的像素宽
		 *	@param[in]	int height Texture的像素高
		 *	@param[in]	unsigned char* initData 初始化Texture的数据，默认为NULL
		 *	@param[in]	int initDataLen 初始化Texture的数据的长度，默认为0
		 *	@param[in]	bool isShared 标示创建的Texture是否可在多个ID3D11Device对象中共享
		 *	@return		YUVTexture_Packed* Texture对象的指针，失败为NULL，当参数不合法或者显卡资源不足时将导致失败
		 */
		IRawFrameTexture* createTexture(PIXFormat pixFmt, int width, int height, 
							unsigned char* initData = NULL, int initDataLen = 0, bool isShared = false);

		IRawFrameTexture* createTexture(PIXFormat pixfmt, int width, int height, TEXTURE_USAGE usage, bool bShared, unsigned char* initData, int dataLen, int pitch);

		IRawFrameTexture* openSharedTexture(IRawFrameTexture* sharedTexture);

		/**
		 *	@name		releaseTexture
		 *	@brief		释放Texture对象
		 *				释放Texture对象中的资源，并删除对象。对象释放将不能再用于显示，否则导致程序出错
		 *	@param[in]	YUVTexture_Packed** texture 指向Texture对象指针的指针，Texture对象释放成功的则该指针被置NULL
		 *	@return		int 0--成功   <0--失败
		 **/
		int releaseTexture(IRawFrameTexture** texture);

		/**
		 *	@name		createVertexBuffer
		 *	@brief		创建顶点信息的ID3D11Buffer资源，该资源在显卡内申请显存
		 *				创建的资源为D3D11_USAGE_IMMUTABLE类型的，不可修改
		 *	@param[in]	int byteCount 创建的资源的总字节数
		 *	@param[in]	const unsigned char* initData 初始化数据，该参数不能为NULL，因为创建的类型是D3D11_USAGE_IMMUTABLE
		 *	@param[in]	int initDataLen 初始化数据的长度，必须>=byteCount
		 *	@return		ID3D11Buffer* 返回创建的ID3D11Buffer对象指针，失败返回NULL
		 **/
		ID3D11Buffer* createVertexBuffer(int byteCount, const unsigned char* initData, int initDataLen);

		/**
		 *	@name		createVertexBuffer
		 *	@brief		创建小标信息的ID3D11Buffer资源，该资源在显卡内申请显存
		 *				创建的资源为D3D11_USAGE_IMMUTABLE类型的，不可修改
		 *	@param[in]	int byteCount 创建的资源的总字节数
		 *	@param[in]	const unsigned char* initData 初始化数据，该参数不能为NULL，因为创建的类型是D3D11_USAGE_IMMUTABLE
		 *	@param[in]	int initDataLen 初始化数据的长度，必须>=byteCount
		 *	@return		ID3D11Buffer* 返回创建的ID3D11Buffer对象指针，失败返回NULL
		 **/
		ID3D11Buffer* createIndexBuffer(int byteCount, const unsigned char* initData, int initDataLen);

		/**
		 *	@name		releaseBuffer
		 *	@brief		释放ID3D11Buffer对象。通过createVertexBuffer等接口创建的ID3D11Buffer资源统一通过该接口释放
		 *				释放ID3D11Buffer对象中的资源，并删除对象。对象释放将不能再用，否则导致程序出错
		 *	@param[in]	ID3D11Buffer** buffer 指向ID3D11Buffer对象指针的指针，ID3D11Buffer对象释放成功的则该指针被置NULL
		 *	@return		int 0--成功   <0--失败
		 **/
		int releaseBuffer(ID3D11Buffer** buffer);

		Texture* openSharedTexture(void* texHandle);////////////////////////////////////////////////////

		/**
		 *	@name		setupBackground
		 *	@brief		启用并设置背景的绘制内容的提供者以及其他参数，引擎将该提供者提供的内容显示在displayReg表示的坐标中
		 *	@param[in]	IDisplayContentProvider* contentProvider 背景的绘制内容的提供者
		 *	@param[in]	const RECT_f& displayReg 该背景内容在坐标系中的位置
		 *	@return		int 0--成功   <0--失败，当已经启用时在调用启用也将返回失败
		 **/
		//int setupBackground(BackgroundComponent* bgComponent);

		/**
		 *	@name		cancleBackground
		 *	@brief		取消背景内容显示
		 *	@return		int 0--成功   <0--失败
		 **/
		//int cancleBackground();

		/**
		 *	@name		draw
		 *	@brief		绘制DisplayElement中保持的显示内容
		 *				DxRender对象正确初始化之后才能调用绘制接口绘制各种资源对象
		 *	@param[in]	DisplayElement* displayElem 显示内容对象，该对象中的顶点、Texture等将会被绘制
		 *	@return		int 0--成功   <0--失败
		 */
		int draw(DisplayElement* displayElem);

		/**
		 *	@name		clear
		 *	@brief		将DxRender中所有显示内容清除，显示背景颜色
		 *	@param[in]	DWORD color 背景颜色
		 *	@return		int 0--成功   <0--失败
		 */
		int clear(DWORD color);

		/**
		 *	@name		clear
		 *	@brief		将DxRender中特定区域的显示内容清除，显示背景颜色
		 *	@param[in]	const RECT_f& clearReg DxRender中的显示区域
		 *	@param[in]	DWORD color 背景颜色
		 *	@return		int 0--成功   <0--失败
		 */
		int clear(const RECT_f& clearReg, DWORD color);

		/**
		 *	@name		present
		 *	@brief		将显卡中的内容在显示设备中呈现
		 *				DxRender对象正确初始化之后才能调用呈现接口
		 *	@param[in]	int type	类型
		 *	@return		int 0--成功   <0--失败
		 */
		int present(int type);

		/**
		 *	@name			lockBackbufferHDC
		 *	@brief			获取BackBuffer的HDC句柄用于显示GDI内容
		 *					参数说明可以参考IDXGISwapChain::GetDC的说明
		 *	@param[in]		BOOL Discard 
		 *	@param[in]		HDC * outHDC 
		 *	@ThreadSafe		No
		 *	@return			int 0--成功 其他--失败
		 **/
		int lockBackbufferHDC(BOOL Discard, HDC* outHDC);
		/**
		 *	@name			unlockBackbufferHDC
		 *	@brief			释放通过lockBackbufferHDC获取的HDC句柄
		 *					调用lockBackbufferHDC并HDC使用完之后必须调用该接口进行释放
		 *	@param[in]		HDC hdc 
		 *	@ThreadSafe		No
		 *	@return			int 0--成功 其他--失败
		 **/
		int unlockBackbufferHDC(HDC hdc);

		/**
		 *	@name			getSnapshot
		 *	@brief			Get the snapshot of the current view for RenderTarget
		 *	@param[out]		unsigned char * pData Destination buffer use to save the picture data of snapshot
		 *	@param[in,out]	UINT & datalen The total length of the destination buffer and save the data length valid when get snapshot success
		 *	@param[out]		int & w Picture width of snapshot
		 *	@param[out]		int & h Picture height of snapshot
		 *	@param[out]		int & pixfmt Picture data pixel format of snapshot
		 *	@param[out]		int & pitch Picture data pitch of snapshot
		 *	@return			bool 0--success other--failed
		 **/
		int getSnapshot(unsigned char* pData, UINT& datalen, int& w, int& h, int& pixfmt, int& pitch);
		int getSnapshot(SharedTexture** outSharedTexture);

		int getWidth();
		int getHeight();
		int resize(int new_width, int new_height);
	private:
		DxRender_D3D11(const DxRender_D3D11&);
		DxRender_D3D11& operator=(const DxRender_D3D11&);

		ID3D11Buffer* createBuffer(int byteCount, const unsigned char* initData, int initDataLen,
									D3D11_USAGE usage, UINT bindFlag);

		int createComputeShader(const std::wstring& fxFileName);
		void releaseComputeShader();

		void createInputLayout();
		void releaseInputLayout();

		void initEffectPass();
		void createEffectPass(PIXFormat pixfmt);
		ID3D11InputLayout* createInputLayout(ID3DX11EffectPass* effectPass);

		ID3D11InputLayout* findInputLayout(PIXFormat pixfmt) const;
		ID3DX11EffectPass* findEffectPass(PIXFormat pixfmt) const;
	private:
		IDXGIAdapter* m_adapter;
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_context;
		IDXGISwapChain* m_swapChain;
		ID3D11RenderTargetView* m_renderTargetView;
		D3D_FEATURE_LEVEL m_curFeatureLv;
		ID3D11Texture2D* m_depthBuffer;
		ID3D11DepthStencilView* m_depthView;
		BasicEffect* m_defaultVideoEffect;
		D3D11_VIEWPORT m_viewport;
		//ID3D11InputLayout* m_InputLayout;

		struct EffectPassInputLayoutPair
		{
			ID3DX11EffectPass* effectPass;
			ID3D11InputLayout* inputLayout;

			EffectPassInputLayoutPair()
				: effectPass(NULL), inputLayout(NULL)
			{
			}
		};
		std::map<PIXFormat, EffectPassInputLayoutPair> m_defaultPassAndInputLayout;

		HWND m_hWnd;
		int m_winWidth;
		int m_winHeight;
		RECT_f m_visibleReg;
		float m_aspectRatio;
		bool m_bEnable4xMsaa;

		XMFLOAT4X4 m_worldBaseTransform;
		XMFLOAT4X4 m_viewTransform;
		XMFLOAT4X4 m_projTransform;
		Material m_material;
		IDXGISurface1* m_bkbufDxgiSurface;
		ID3D11BlendState* m_TransparentBS;

		//BackgroundComponent* m_backgroundComponent;
	};
}
#endif

#endif //_zRENDER_DXRENDER_D3D11_H_