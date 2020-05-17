/**
 *	@author		zhuqingquan
 *	@date		2014-10-17
 *	@name		DisplayElement.h
 *	@brief		画面中显示的内容的逻辑类。在类中包含了内容显示所需要的所有信息，如顶点信息、Texture、world坐标的转换矩阵、使用的Shader
 */

#pragma once
#ifndef _ZRENDER_DISPLAYELEMENT_H_
#define _ZRENDER_DISPLAYELEMENT_H_

#include "DxZRenderDLLDefine.h"
#include <memory>
#ifdef _WINDOWS

#include "IDisplayContentProvider.h"
#include "Vertex.h"
#include "DxRenderCommon.h"
#include "Effects.h"
#include "ElemDsplModel.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace zRender
{
	class DxRender_D3D11;
	class BasicEffect;
	class RectangleDataCtxInitializer;
	class ElementDrawingContext;
	class ElementMetaData;
	class TextureFasterCopyHelper;

	//template<> class ElemDsplModel<BasicEffect>;

	/**
	 *	@name	DisplayElement
	 *	@brief	显示内容的资源类
	 **/
	class DX_ZRENDER_EXPORT_IMPORT DisplayElement
	{
	public:
		/**
		 *	@name		DisplayElement
		 *	@brief		构造函数，构造对象，不创建任何资源
		 *	@param[in]	DxRender* dxRender DxRender对象指针
		 **/
		DisplayElement(DxRender* dxRender);

		/**
		 *	@name		~DisplayElement
		 *	@brief		析构函数，对象中管理的相关资源将被释放
		 **/
		virtual ~DisplayElement() = 0;

		/**
		 *	@name		setDisplayRegion
		 *	@brief		设置该显示内容在显示引擎坐标系中的位置
		 *				用户可通过重新设置该位置达到移动显示内容的作用
		 *	@param[in]	const RECT_f& displayReg	表示显示区域的的坐标系位置
		 *	@param[in]	float zIndex 当前显示原始所在顶点的Z坐标偏移
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		 **/
		int move(const RECT_f& displayReg, float zIndex);//处理移动，设置内容的显示位置

		/**
		 *	@name		setDisplayZIndex
		 *	@brief		设置该显示内容在显示引擎坐标系中的位置的Z坐标
		 *				用户可通过重新设置该位置达到移动显示内容Z坐标的作用
		 *	@param[in]	float zIndex 当前显示原始所在顶点的Z坐标偏移
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		**/
		int setZIndex(float zIndex);

		float getZIndex() const;

		/**
		 *	@name		setTextureDataSource
		 *	@brief		重新设置Texture的纹理数据提供对象
		 *				在draw时如果该对象已设置，则从该对象中拷贝纹理数据到D3D11的Texture显存中
		 *	@param[in]	TextureDataSource* dataSrc 数据来源，来源的改变不会导致资源重新创建
		 *				dataSrc为NULL时，则清楚Texture的数据源，Texture不会更新，此时参数textureReg无意义，不检查，不保存
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		 **/
		int setTextureDataSource(TextureDataSource* dataSrc);

		/**
		 *	@name		createRenderResource
		 *	@brief		创建Render渲染绘制所需要的系统资源
		 *				创建的资源包括：顶点Buffer，Index Buffer，Texture，Shader
		 *				如果该对象未初始化则失败，如果资源之前已经创建，而修改了配置参数（如Texture的宽高）
		 *				则之前创建的资源先释放，然后再创建新的资源。所以应该先确保不再使用老的资源，避免多线程问题。
		 *				只有在所有需要创建或者重新创建的资源都创建成功才会返回成功。
		 *	@return		int	 0--成功  <0--失败
		 **/
		int createRenderResource();

		/**
		 *	@name		releaseRenderResource
		 *	@brief		释放已经创建的资源
		 *	@return		int	 0--成功  <0--失败
		 **/
		void releaseRenderResource();

		DxRender* getParentDxRender() const { return m_dxRender; }
		ID3D11Buffer* getVertexBuffer() const { return m_VertexBuf; }
		ID3D11Buffer* getIndexBuffer() const { return m_IndexBuf; }

		/**
		 *	@name		updateTexture
		 *	@brief		将TextureDataSource中的数据更新到Texture内，需要先调用setTextureDataSource。
		 *				必须在调用了createRenderResource成功后调用该接口才有意义，如果未设置Texture，该接口什么都不做
		 *				该接口从TextureDataSource对象中将内存拷贝到Texture的显存中，如果TextureDataSource内容未更新，则不做任何拷贝
		 *	@return		int 0--成功  <0--失败
		 **/
		int updateTexture();

		/**
		 *	@name		isValid
		 *	@brief		判断该显示对象是否提供了正确显示所需的资源与信息
		 *	@return		bool true--可以显示  false--不可显示
		 **/
		virtual bool isValid() const;

		/**
		 *	@name			setAlpha
		 *	@brief			设置显示内容透明度，有效范围（1.0f -- 0.0f）
		 *	@param[in]		float alpha 透明度，1.0f为不透明， 0.0f为全透明
		 **/
		void setAlpha(float alpha);
		/**
		 *	@name			getAlpha
		 *	@brief			获取当前设置的透明度
		 *	@return			float 
		 **/
		float getAlpha() const;
		void enableTransparent(bool enable);
		bool isEnableTransparent() const;

		int draw();

		ElementMetaData* getMetaData() const { return m_MetaData; }

	protected:

		/**
		 *	@name		init
		 *	@brief		初始化，完成MetaData、DrawingContext的初始化
						构造函数中将直接调用这个方法进行环境初始化
		 *	@return		int 0--成功  其他--失败
		 **/
		virtual int init(DxRender* dxRender) = 0;

		/**
		 *	@name		deinit
		 *	@brief		释放init调用中创建的资源
		 *	@return		int 0--成功  其他--失败
		 **/
		virtual void deinit() = 0;

		RectangleDataCtxInitializer* m_DataCtxInitializer;
		ElementDrawingContext* m_DrawingContext;
		ElementMetaData* m_MetaData;

		DxRender* m_dxRender;
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_context;

		bool m_isVertexInfoUpdated;
		bool m_isTextureUpdated;
	private:
		DisplayElement(const DisplayElement& rObj);
		DisplayElement& operator=(const DisplayElement& robj);

		//创建与释放资源
		int createTextureResource();
		int createTextureResource(TextureDataSource* dataSrc);
		void releaseTextureResource();
		int createVertexBuffer();
		int releaseVertexBuffer();
		int createIndexBuffer();
		int releaseIndexBuffer();
		void setupApplyingTextureList();

		struct TextureResContext
		{
			TextureResContext(ITextureResource* tex, TextureFasterCopyHelper* texHelper)
				: texture(tex), textureHelper(texHelper)
				, identifyForData(0), identifyForFormat(0)
			{

			}

			TextureResContext()
				: texture(nullptr), identifyForData(0), identifyForFormat(0)
			{

			}

			ITextureResource* texture;
			TextureFasterCopyHelper* textureHelper;
			unsigned int identifyForData;
			unsigned int identifyForFormat;
		};

		std::vector<ITextureResource*> m_applyingTexture;
		std::vector<TextureResContext> m_textureRes;
		ID3D11Buffer*		m_VertexBuf;
		ID3D11Buffer*		m_IndexBuf;
	};
}
#pragma warning(pop)

#endif

#endif	//_zRENDER_DISPLAYELEMENT_H_