/**
 *	@file		d3d11StateHelper.h
 *	@brief		实现将ID3D11DeviceContext中的管线状态（state of dx pipeline)保存到对象中或者从对象中恢复
 *	@author		zhuqingquan
 **/
#pragma once
#ifndef _D3D11_STATE_HELPER_H_
#define _D3D11_STATE_HELPER_H_

#include "D3D11.h"
#include <inttypes.h>

#define MAX_RENDER_TARGETS             D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT
#define MAX_SO_TARGETS                 4
#define MAX_CLASS_INSTS                256

namespace zRender
{
	struct d3d11_state {
		ID3D11GeometryShader           *geom_shader;
		ID3D11InputLayout              *vertex_layout;
		D3D11_PRIMITIVE_TOPOLOGY       topology;
		ID3D11Buffer                   *vertex_buffer;
		UINT                           vb_stride;
		UINT                           vb_offset;
		ID3D11BlendState               *blend_state;
		float                          blend_factor[4];
		UINT                           sample_mask;
		ID3D11DepthStencilState        *zstencil_state;
		UINT                           zstencil_ref;
		ID3D11RenderTargetView         *render_targets[MAX_RENDER_TARGETS];
		ID3D11DepthStencilView         *zstencil_view;
		ID3D11SamplerState             *sampler_state;
		ID3D11PixelShader              *pixel_shader;
		ID3D11ShaderResourceView       *resource;
		ID3D11RasterizerState          *raster_state;
		UINT                           num_viewports;
		D3D11_VIEWPORT                 *viewports;
		ID3D11Buffer                   *stream_output_targets[MAX_SO_TARGETS];
		ID3D11VertexShader             *vertex_shader;
		ID3D11ClassInstance            *gs_class_instances[MAX_CLASS_INSTS];
		ID3D11ClassInstance            *ps_class_instances[MAX_CLASS_INSTS];
		ID3D11ClassInstance            *vs_class_instances[MAX_CLASS_INSTS];
		UINT                           gs_class_inst_count;
		UINT                           ps_class_inst_count;
		UINT                           vs_class_inst_count;
	};

	/**
	 * @name		d3d11_save_state
	 * @brief		保存ID3D11DeviceContext中当前的管线状态
	 *				需要与d3d11_restore_state成对调用
	 * @param[out]	struct d3d11_state *state 用于保存获取到的所有的管线状态
	 * @param[in]	ID3D11DeviceContext* d3d11Context 需要保存的ID3D11DeviceContext
	 * @return		true--成功 false--失败
	 **/
	bool d3d11_save_state(struct d3d11_state *state, ID3D11DeviceContext* d3d11Context);

	/**
	* @name			d3d11_restore_state
	* @brief		将保存的管线状态恢复到ID3D11DeviceContext中
	*				需要与d3d11_save_state成对调用
	* @param[in]	struct d3d11_state *state 保存着获取到的所有的管线状态
	* @param[in]	ID3D11DeviceContext* d3d11Context 需要恢复状态的ID3D11DeviceContext
	* @return		true--成功 false--失败
	**/
	bool d3d11_restore_state(struct d3d11_state *state, ID3D11DeviceContext* d3d11Context);
}

#endif //_D3D11_STATE_HELPER_H_
