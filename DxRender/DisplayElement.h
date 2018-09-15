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
#include "IRawFrameTexture.h"
#include "DxRenderCommon.h"
#include "Effects.h"
#include "ElemDsplModel.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace zRender
{
	class Texture;
	class DxRender_D3D11;
	class BasicEffect;
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
		 *	@param[in]	DxRender_D3D11* dxRender DxRender对象指针
		 *	@param[in]	ID3D11Device* d3dDevice D3DDevice对象指针
		 *	@param[in]	ID3D11DeviceContext* contex 与d3dDevice参数对应的设备上下文
		 **/
		DisplayElement(DxRender_D3D11* dxRender, ID3D11Device* d3dDevice, ID3D11DeviceContext* contex);

		/**
		 *	@name		~DisplayElement
		 *	@brief		析构函数，对象中管理的相关资源将被释放
		 **/
		~DisplayElement();

		/**
		 *	@name		setDisplayRegion
		 *	@brief		设置该显示内容在显示引擎坐标系中的位置
		 *				用户可通过重新设置该位置达到移动显示内容的作用
		 *	@param[in]	const RECT_f& displayReg	表示显示区域的的坐标系位置
		 *	@param[in]	float zIndex 当前显示原始所在顶点的Z坐标偏移
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		 **/
		int setDisplayRegion(const RECT_f& displayReg, float zIndex);//处理移动，设置内容的显示位置

		/**
		 *	@name		setDisplayZIndex
		 *	@brief		设置该显示内容在显示引擎坐标系中的位置的Z坐标
		 *				用户可通过重新设置该位置达到移动显示内容Z坐标的作用
		 *	@param[in]	float zIndex 当前显示原始所在顶点的Z坐标偏移
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		**/
		int setDisplayZIndex(float zIndex);

		float getZIndex() const;

		/**
		 *	@name		setTexture
		 *	@brief		重新设置Texture的相关参数信息
		 *				在此不会创建释放资源，在createRenderResource会检查释放需要释放并重新创建Texture资源
		 *	@param[in]	YUVFormat_Packed pixfmt	Texture像素格式，像素格式改变不一定会导致资源重新创建，如YUYV->UYVY
		 *	@param[in]	int width	Texture的宽，width值的改变将导致资源重新创建，值域为>0
		 *	@param[in]	int height	Texture的高，height值的改变将导致资源重新创建，值域为>0
		 *	@param[in]	TextureDataSource* dataSrc 数据来源，来源的改变不会导致资源重新创建
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		 **/
		int setTexture(PIXFormat pixfmt, int width, int height);

		/**
		 *	@name		setTextureDataSource
		 *	@brief		重新设置Texture的纹理数据提供对象
		 *				在draw时如果该对象已设置，则从该对象中拷贝纹理数据到D3D11的Texture显存中
		 *	@param[in]	TextureDataSource* dataSrc 数据来源，来源的改变不会导致资源重新创建
		 *				dataSrc为NULL时，则清楚Texture的数据源，Texture不会更新，此时参数textureReg无意义，不检查，不保存
		 *	@param[in]	const RECT_f& textureReg dataSrc中的区域，该DisplayElement对象只显示该区域中的数据。相对坐标。取值范围[0~1]
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		 **/
		int setTextureDataSource(TextureDataSource* dataSrc, const RECT_f& textureReg);

		/**
		 *	@name		setVertex
		 *	@brief		重新设置顶点相关信息
		 *				存在两种情况，1-顶点信息全部更新，2-之前的顶点信息部分更新
		 *	@param		VertexVector* vertexInfo	顶点信息，支持设置不同的顶点信息对象或者设置同一个对象
		 *	@return		int 0--成功 <0--失败	该参数不合法时失败
		 **/
		int setVertex(VertexVector* vertexInfo);

		/**
		 *	@name		getVertex
		 *	@brief		获取顶点相关信息对象
		 *	@return		VertexVector* 返回顶点相关信息对象，NULL--失败
		 **/
		VertexVector* getVertex() const { return m_curVerVec; }

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
		int releaseRenderResource();

		DxRender_D3D11* getParentDxRender() const { return m_dxRender; }
		IRawFrameTexture* getTexture() const { return m_texture; }
		ID3D11Buffer* getVertexBuffer() const { return m_VertexBuf; }
		ID3D11Buffer* getIndexBuffer() const { return m_IndexBuf; }
		DXGI_FORMAT getIndexBufferFormat() const { return m_IndexFmt; }
		XMFLOAT4X4 getWorldTransformMatrices() const	{return m_WorldTransformMat;}

		/**
		 *	@name		updateTexture
		 *	@brief		更新Texture的内容
		 *				必须在调用了createRenderResource成功后调用该接口才有意义，如果为设置Texture，该接口什么都不做
		 *				该接口从TextureDataSource对象中将内存拷贝到Texture的显存中，如果TextureDataSource内容未更新，则不做任何拷贝
		 *	@param[in,out]	int& identify	当前已经绘制的Texture的标识符，如果Texture的内容更新成功，则该参数将被修改
		 *	@return		int 0--成功  <0--失败
		 **/
		int updateTexture(int& identify);

		/**
		 *	@name		isValid
		 *	@brief		判断该显示对象是否提供了正确显示所需的资源与信息
		 *	@return		bool true--可以显示  false--不可显示
		 **/
		bool isValid() const;

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
		bool isEnableTransparent() const { return m_isEnableTransparent;}

		int createDsplModel(ElemDsplModel<BasicEffect>** ppModel)
		{
			if (ppModel == NULL)	return -1;
			ElemDsplModel<BasicEffect>* model = new ElemDsplModel<BasicEffect>();
			if (NULL == model)	return -2;
			*ppModel = model;
			return 0;
		}

		void ReleaseDsplModel(ElemDsplModel<BasicEffect>** ppModel)
		{
			if (ppModel && *ppModel)
			{
				ElemDsplModel<BasicEffect>* model = *ppModel;
				delete model;
				*ppModel = NULL;
			}
		}

		bool setDsplModel(ElemDsplModel<BasicEffect>* model)
		{
			if (NULL == model)	return false;
			m_dsplModel = model;
			return true;
		}

		ElemDsplModel<BasicEffect>* getDsplModel() const { return m_dsplModel; }
		int draw();
		//////////////////////////支持打开共享显存用于显示///////////////////////////////
		int openSharedTexture(IRawFrameTexture* sharedTexture);
	private:
		DisplayElement(const DisplayElement& rObj);
		DisplayElement& operator=(const DisplayElement& robj);

		//创建与释放资源
		int createTextureResource();
		int releaseTexTureResource();
		int createVertexBuffer();
		int releaseVertexBuffer();
		int createIndexBuffer();
		int releaseIndexBuffer();

		DxRender_D3D11* m_dxRender;
		ID3D11Device*	m_device;
		ID3D11DeviceContext* m_context;

		ElemDsplModel<BasicEffect>* m_dsplModel;

		VertexVector* m_curVerVec;
		bool m_isVertexInfoUpdated;
		PIXFormat m_TexFmt;
		int	m_TexWidth;
		int m_TexHeight;
		TextureDataSource* m_TexDataSrc;
		RECT_f	m_TexEffectiveReg;
		bool m_isTextureUpdated;

		IRawFrameTexture*	m_texture;//std::shared_ptr<YUVTexture_Packed*>
		ID3D11Buffer*		m_VertexBuf;//std::shared_ptr<ID3D11Buffer*>
		ID3D11Buffer*		m_IndexBuf;//std::shared_ptr<ID3D11Buffer*>
		DXGI_FORMAT			m_IndexFmt;//
		XMFLOAT4X4			m_WorldTransformMat;
		float				m_alpha;
		bool				m_isEnableTransparent;
	};
}
#pragma warning(pop)

#endif

#endif	//_zRENDER_DISPLAYELEMENT_H_