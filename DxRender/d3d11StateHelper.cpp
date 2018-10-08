#include "d3d11StateHelper.h"

using namespace zRender;

bool zRender::d3d11_save_state(struct d3d11_state *state, ID3D11DeviceContext* d3d11Context)
{
	if (state == nullptr || d3d11Context == nullptr)
		return false;
	state->gs_class_inst_count = MAX_CLASS_INSTS;
	state->ps_class_inst_count = MAX_CLASS_INSTS;
	state->vs_class_inst_count = MAX_CLASS_INSTS;

	d3d11Context->GSGetShader(&state->geom_shader,
		state->gs_class_instances,
		&state->gs_class_inst_count);
	d3d11Context->IAGetInputLayout(&state->vertex_layout);
	d3d11Context->IAGetPrimitiveTopology(&state->topology);
	d3d11Context->IAGetVertexBuffers(0, 1, &state->vertex_buffer,
		&state->vb_stride, &state->vb_offset);
	d3d11Context->OMGetBlendState(&state->blend_state, state->blend_factor,
		&state->sample_mask);
	d3d11Context->OMGetDepthStencilState(&state->zstencil_state,
		&state->zstencil_ref);
	d3d11Context->OMGetRenderTargets(MAX_RENDER_TARGETS,
		state->render_targets, &state->zstencil_view);
	d3d11Context->PSGetSamplers(0, 1, &state->sampler_state);
	d3d11Context->PSGetShader(&state->pixel_shader,
		state->ps_class_instances,
		&state->ps_class_inst_count);
	d3d11Context->PSGetShaderResources(0, 1, &state->resource);
	d3d11Context->RSGetState(&state->raster_state);
	d3d11Context->RSGetViewports(&state->num_viewports, nullptr);
	if (state->num_viewports) {
		state->viewports = (D3D11_VIEWPORT*)malloc(
			sizeof(D3D11_VIEWPORT) * state->num_viewports);
		d3d11Context->RSGetViewports(&state->num_viewports,
			state->viewports);
	}
	d3d11Context->SOGetTargets(MAX_SO_TARGETS,
		state->stream_output_targets);
	d3d11Context->VSGetShader(&state->vertex_shader,
		state->vs_class_instances,
		&state->vs_class_inst_count);
	return true;
}

static inline void safe_release(IUnknown *p)
{
	if (p) p->Release();
}

#define SO_APPEND ((UINT)-1)

bool zRender::d3d11_restore_state(struct d3d11_state *state, ID3D11DeviceContext* d3d11Context)
{
	if (nullptr == state || nullptr == d3d11Context)
		return false;
	UINT so_offsets[MAX_SO_TARGETS] =
	{ SO_APPEND, SO_APPEND, SO_APPEND, SO_APPEND };

	d3d11Context->GSSetShader(state->geom_shader,
		state->gs_class_instances,
		state->gs_class_inst_count);
	d3d11Context->IASetInputLayout(state->vertex_layout);
	d3d11Context->IASetPrimitiveTopology(state->topology);
	d3d11Context->IASetVertexBuffers(0, 1, &state->vertex_buffer,
		&state->vb_stride, &state->vb_offset);
	d3d11Context->OMSetBlendState(state->blend_state, state->blend_factor,
		state->sample_mask);
	d3d11Context->OMSetDepthStencilState(state->zstencil_state,
		state->zstencil_ref);
	d3d11Context->OMSetRenderTargets(MAX_RENDER_TARGETS,
		state->render_targets,
		state->zstencil_view);
	d3d11Context->PSSetSamplers(0, 1, &state->sampler_state);
	d3d11Context->PSSetShader(state->pixel_shader,
		state->ps_class_instances,
		state->ps_class_inst_count);
	d3d11Context->PSSetShaderResources(0, 1, &state->resource);
	d3d11Context->RSSetState(state->raster_state);
	d3d11Context->RSSetViewports(state->num_viewports, state->viewports);
	d3d11Context->SOSetTargets(MAX_SO_TARGETS,
		state->stream_output_targets, so_offsets);
	d3d11Context->VSSetShader(state->vertex_shader,
		state->vs_class_instances,
		state->vs_class_inst_count);
	safe_release(state->geom_shader);
	safe_release(state->vertex_layout);
	safe_release(state->vertex_buffer);
	safe_release(state->blend_state);
	safe_release(state->zstencil_state);
	for (size_t i = 0; i < MAX_RENDER_TARGETS; i++)
		safe_release(state->render_targets[i]);
	safe_release(state->zstencil_view);
	safe_release(state->sampler_state);
	safe_release(state->pixel_shader);
	safe_release(state->resource);
	safe_release(state->raster_state);
	for (size_t i = 0; i < MAX_SO_TARGETS; i++)
		safe_release(state->stream_output_targets[i]);
	safe_release(state->vertex_shader);
	for (size_t i = 0; i < state->gs_class_inst_count; i++)
		state->gs_class_instances[i]->Release();
	for (size_t i = 0; i < state->ps_class_inst_count; i++)
		state->ps_class_instances[i]->Release();
	for (size_t i = 0; i < state->vs_class_inst_count; i++)
		state->vs_class_instances[i]->Release();
	free(state->viewports);
	memset(state, 0, sizeof(*state));
}