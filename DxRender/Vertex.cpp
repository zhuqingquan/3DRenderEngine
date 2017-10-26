#include "Vertex.h"

using namespace zRender;

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Basic32[4] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR",	 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

VertexVector::VertexVector(D3D11_PRIMITIVE_TOPOLOGY topology)
	: m_topology(topology)
{
}

VertexVector::~VertexVector()
{
	m_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

int VertexVector::addVertexs(const Vertex* vertexList, int vertexCount, const UINT* indexList, int indexCount)
{
	if(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED==m_topology)
		return -1;
	if(vertexList==NULL || indexList==NULL || vertexCount<=0 || indexCount<=0)
	{
#ifdef _DEBUG
		printf("Error in VertexVector::addVertexs : param invalid.(vl=%d, vc=%d, il=%d, ic=%d)\n",
			(int)vertexList, vertexCount, (int)indexList, indexCount);
#endif
		return -2;
	}
	for(int vi = 0; vi<vertexCount; vi++)
	{
		Vertex* vt = (Vertex*)vertexList + vi;
		m_Vertexs.push_back(*vt);
	}
	for(int ii=0; ii<indexCount; ii++)
	{
		UINT* index = (UINT*)indexList + ii;
		m_Indexs.push_back(*index);
	}
	return 0;
}

int VertexVector::clearVertexs()
{
	if(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED==m_topology)
		return -1;
	m_Vertexs.clear();
	m_Indexs.clear();
	return 0;
}

const unsigned char* VertexVector::getVertexData(int& dataLen) const
{
	if(m_Vertexs.empty())
	{
		dataLen = 0;
		return NULL;
	}
	dataLen = m_Vertexs.size() * sizeof(Vertex);
	return (unsigned char*)&m_Vertexs[0];
}

const unsigned char* VertexVector::getIndexData(int& dataLen) const
{
	if(m_Indexs.empty())
	{
		dataLen = 0;
		return NULL;
	}
	dataLen = m_Indexs.size() * sizeof(UINT);
	return (unsigned char*)&m_Indexs[0];
}

Vertex* VertexVector::getVertex(int index) const
{
	return index>(int)m_Vertexs.size() ? NULL : const_cast<Vertex*>(&m_Vertexs[index]);
}

UINT	VertexVector::getIndex(int index) const
{
	return index>(int)m_Indexs.size() ? -1 : m_Indexs[index];
}