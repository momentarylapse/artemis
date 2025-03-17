//
// Created by michi on 09.03.25.
//

#include "MeshRenderer.h"
#include <Session.h>
#include <view/DrawingHelper.h>
#include <view/MultiView.h>
#include <y/graphics-impl.h>

namespace graph {

	MeshRenderer::MeshRenderer(Session* s) : RendererNode(s, "MeshRenderer") {
		material = new Material(s->resource_manager);
	}

	MeshRenderer::~MeshRenderer() = default;

	void MeshRenderer::process() {
		auto mesh = in_mesh.value();
		if (!mesh)
			return;

		if (!vertex_buffer)
			vertex_buffer = new VertexBuffer("3f,3f,2f");
		mesh->build(vertex_buffer.get());

		material->roughness = roughness();
		material->metal = metal();
		material->albedo = albedo();
		material->emission = emission();

		out_draw({});
	}

void MeshRenderer::draw_win(const RenderParams& params, MultiViewWindow* win) {
	auto vb = vertex_buffer.get();
	if (!vb)
		return;

	session->drawing_helper->draw_mesh(params, win->rvd, mat4::ID, vb, material.get());
}


} // graph