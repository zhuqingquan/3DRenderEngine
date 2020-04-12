/**
 *	@author		zhuqingquan
 *	@date		2014-10-20
 *	@name		Vertex.h
 *	@brief		顶点Vertex结构体内容的定义，以及对应的InputLayout结构体的定义
 *				该结构体信息与fx文件中对应的输入顶点信息的布局对应，如修改必须同时修改
 */

#pragma once
#ifndef _ZRENDER_VERTEX_H_
#define _ZRENDER_VERTEX_H_

#include <vector>
#include "DxZRenderDLLDefine.h"

#ifdef _WINDOWS
#include <Windows.h>
#include <d3d11.h>
#include <xnamath.h>
#endif //_WINDOWS

#pragma warning(push)
#pragma warning(disable:4251)

namespace zRender
{
#ifdef _WINDOWS
	/**
	 *	@name	Vertex
	 *	@brief	顶点的所有信息
	 **/
	struct Vertex
	{
		XMFLOAT3 Pos;		//顶点坐标，(x,y,z),这里使用的物体本身的坐标系
		XMFLOAT3 Normal;	//暂时无用
		XMFLOAT2 Tex;		//该顶点对应的在Texture中的位置，如果没有用于渲染的ps Effect不使用Texture，则忽略
		XMFLOAT4 Color;		//在使用纯色渲染时使用该成员
	};

	/**
	 *	@name	InputLayoutDesc
	 *	@brief	Vertex结构体内存布局信息的描述对象
	 **/
	class InputLayoutDesc
	{
	public:
		// Init like const int A::a[4] = {0, 1, 2, 3}; in .cpp file.
		static const D3D11_INPUT_ELEMENT_DESC Basic32[4];
	};
#endif //_WINDOWS

	/**
	 *	@name	VertexVector
	 *	@brief	Vertex数组，保持一组顶点信息，并保存该数组的拓扑结构以及与该拓扑结构对应的index信息
	 *			所有拓扑结构相同的的顶点信息可以添加都同一个数组中
	 **/
	class DX_ZRENDER_EXPORT_IMPORT VertexVector
	{
	public:
		/**
		 *	@name		VertexVector
		 *	@brief		构造函数，构造一个存储特定拓扑类型的顶点信息的数组
		 *	@param[in]	D3D11_PRIMITIVE_TOPOLOGY topology 顶点的拓扑连接类型
		 **/
		VertexVector(D3D11_PRIMITIVE_TOPOLOGY topology);

		/**
		 *	@name		VertexVector
		 *	@brief		析构函数
		 **/
		~VertexVector();

		/**
		 *	@name		addVertexs
		 *	@brief		添加顶点信息以及与之对应的下标信息。添加的信息将在本对象中拷贝一份，下标信息将被修改
		 *	@param[in]	const Vertex* vertexList 顶点信息的数组
		 *	@param[in]	int vertexCount 顶点信息的数组的长度
		 *	@param[in]	const UINT* indexList 下标信息的数组
		 *	@param[in]	int indexCount 下标信息的数组的长度
		 *	@return		int 0--成功	<0--失败，参数不合法或者无法创建资源时导致添加失败
		 **/
		int addVertexs(const Vertex* vertexList, int vertexCount, 
						const UINT* indexList, int indexCount);

		/**
		 *	@name		clearVertexs
		 *	@brief		删除顶点信息以及与之对应的下标信息。
		 *	@return		int 0--成功	<0--失败
		 **/
		int clearVertexs();

		/**
		 *	@name		getVertexDataAddr
		 *	@brief		获取保存着顶点信息的内存的首地址
		 *	@param[out]	int& dataLen 输出参数，输出内存的字节数
		 *	@return		const unsigned char* 保存着顶点信息的内存的首地址, NULL--无顶点信息
		 **/
		const unsigned char* getVertexData(int& dataLen) const;

		/**
		 *	@name		getIndexDataAddr
		 *	@brief		获取保存着下标信息的内存的首地址
		 *	@param[out]	int& dataLen 输出参数，输出内存的字节数
		 *	@return		const unsigned char* 保存着下标信息的内存的首地址, NULL--无下标信息
		 **/
		const unsigned char* getIndexData(int& dataLen) const;

		size_t getVertexCount() const { return m_Vertexs.size(); }
		size_t getIndexCount() const  { return m_Indexs.size(); }

		Vertex* getVertex(int index) const;
		UINT	getIndex(int index) const;

		D3D11_PRIMITIVE_TOPOLOGY getPrimitiveTopology() const { return m_topology; }
		DXGI_FORMAT getIndexBufferFormat() const { return m_IndexFmt; }

	private:
		D3D11_PRIMITIVE_TOPOLOGY m_topology;
		DXGI_FORMAT			m_IndexFmt = DXGI_FORMAT_R32_UINT;//
		std::vector<UINT>	m_Indexs;
		std::vector<Vertex> m_Vertexs;
	};
}
#pragma warning(pop)

#endif //_ZRENDER_VERTEX_H_