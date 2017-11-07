/**
 *	@author		zhuqingquan
 *	@date		2014-10-17
 *	@name		DxRender.h
 *	@brief		Interface of the display engine
 */

#pragma once
#ifndef _ZRENDER_DXRENDER_H_
#define _ZRENDER_DXRENDER_H_

#ifdef _WINDOWS
#include <Windows.h>
#endif
#include "DxZRenderDLLDefine.h"
#include "DxRenderCommon.h"

namespace zRender
{
#ifdef _WINDOWS
	class DxRender_D3D11;		//如果在Windows下编译，则使用DxRender_D3D11对象作为底层实现
	class DisplayElement;
	class YUVTexture_Packed;
#endif

	class BackgroundComponent;
	class IDisplayContentProvider;
	class SharedTexture;
	class IRawFrameTexture;
	class TextureResource;

	/**
	 *	@name		DxRender
	 *	@brief		显示引擎的核心模块的接口类，提供创建显示资源的接口，并执行显示绘制操作
	 *				具体的实现由代理实现类来完成
	 **/
	class DX_ZRENDER_EXPORT_IMPORT DxRender
	{
	public:
		/**
		 *	@name	DxRender
		 *	@brief	构造方法，对象创建还需调用init初始化资源
		 */
		DxRender();

		/**
		 *	@name	~DxRender
		 *	@brief	析构方法，释放所有创建的资源
		 */
		~DxRender();

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
		RECT_f getVisibleRegion() const;

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

		IRawFrameTexture * createTexture(PIXFormat pixfmt, int width, int height, TEXTURE_USAGE usage, bool bShared, unsigned char * initData, int dataLen, int pitch);

		IRawFrameTexture* openSharedTexture(IRawFrameTexture* sharedTexture);

		void releaseTexture(IRawFrameTexture** rawFrameTexture);

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
		//YUVTexture_Packed* createTexture(YUVFormat_Packed pixFmt, int width, int height, 
		//					unsigned char* initData = NULL, int initDataLen = 0, bool isShared = false);

		/**
		 *	@name		releaseTexture
		 *	@brief		释放Texture对象
		 *				释放Texture对象中的资源，并删除对象。对象释放将不能再用于显示，否则导致程序出错
		 *	@param[in]	YUVTexture_Packed** texture 指向Texture对象指针的指针，Texture对象释放成功的则该指针被置NULL
		 *	@return		int 0--成功   <0--失败
		 **/
		//int releaseTexture(YUVTexture_Packed** texture);

		/**
		 *	@name		setupBackground
		 *	@brief		启用并设置背景的绘制内容的提供者以及其他参数，引擎将该提供者提供的内容显示在displayReg表示的坐标中
		 *				非线程安全
		 *	@param[in]	IDisplayContentProvider* contentProvider 背景的绘制内容的提供者
		 *	@param[in]	const RECT_f& displayReg 该背景内容在坐标系中的位置
		 *	@return		int 0--成功   <0--失败，当已经启用时在调用启用也将返回失败
		 **/
		int setupBackground(IDisplayContentProvider* contentProvider, const RECT_f& displayReg);

		/**
		 *	@name		cancleBackground
		 *	@brief		取消背景内容显示
		 *				非线程安全
		 *	@return		int 0--成功   <0--失败
		 **/
		int cancleBackground();

		/**
		 *	@name		drawBackground
		 *	@brief		绘制背景
		 *				非线程安全
		 *	@return		int 0--成功   <0--失败，为设置启动背景或者背景的内容无法显示
		 **/
		int drawBackground();

		/**
		 *	@name		draw
		 *	@brief		绘制DisplayElement中保持的显示内容
		 *				DxRender对象正确初始化之后才能调用绘制接口绘制各种资源对象
		 *	@param[in]	DisplayElement* displayElem 显示内容对象，该对象中的顶点、Texture等将会被绘制
		 *	@return		int 0--成功   <0--失败
		 */
		int draw(DisplayElement* displayElem);

		/**
		 *	@name		present
		 *	@brief		将显卡中的内容在显示设备中呈现
		 *				DxRender对象正确初始化之后才能调用呈现接口
		 *	@param[in]	int type	类型
		 *	@return		int 0--成功   <0--失败
		 */
		int present(int type);

		/**
		 *	@name		clear
		 *	@brief		将DxRender中所有显示内容清除，显示背景颜色
		 *	@param[in]	DWORD color 背景颜色
		 *	@return		int 0--成功   <0--失败
		 */
		int clear(DWORD color);

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
		/**
		*	@name			getSnapshot
		*	@brief			将RenderTarget中的内容拷贝出来
		*					当有调用createOffscreenRenderTarget成功时，通过参数判断时需要获取Offscreen texture还是Backbuffer的Texture
		*	@param[in]		TEXTURE_USAGE usage The texture usage type for saved the snapshot
		*	@param[in]		bool bShared Is the result texture can be shared
		*	@param[in]		bool fromOffscreenTexture True for get the snapshot of Offscreen texture, otherwise get Backbuffer snapshot
		*					if createOffscreenRenderTarget never called success, allways get the snapshot of Offscreen texture
		*	@return			TextureResource* result of Texture saved the snapshot.NULL for failed.
		**/
		TextureResource* getSnapshot(TEXTURE_USAGE usage, bool bShared, bool fromOffscreenTexture);

		int getWidth();
		int getHeight();
		int resize(int new_width, int new_height);

		int createOffscreenRenderTarget(int width, int height);
		void releaseOffscreenRenderTarget();
	private:
		DxRender(const DxRender&);
		DxRender& operator=(const DxRender&);

#ifdef _WINDOWS
		DxRender_D3D11* m_renderImp;
#endif
		BackgroundComponent* m_background;
	};
}

#endif //_zRENDER_DXRENDER_H_