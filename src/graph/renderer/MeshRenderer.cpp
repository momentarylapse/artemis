//
// Created by michi on 09.03.25.
//

#include "MeshRenderer.h"
#include <Session.h>
#include <view/DrawingHelper.h>
#include <view/MultiView.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/base.h>

namespace artemis::graph {

base::optional<Box> mesh_bounding_box(const PolygonMesh& mesh) {
	if (mesh.vertices.num == 0)
		return base::None;
	Box b = {mesh.vertices[0].pos, mesh.vertices[0].pos};
	for (const auto& v: mesh.vertices)
		b = b or Box{v.pos, v.pos};
	return b;
}

MeshRenderer::MeshRenderer(Session* s) : RendererNode(s, "MeshRenderer") {
	material = new yrenderer::Material(s->ctx);
	material->textures.add(s->ctx->tex_white);
}

MeshRenderer::~MeshRenderer() = default;

void MeshRenderer::on_process() {
	auto mesh = in_mesh.value();
	if (!mesh)
		return;

	if (!vertex_buffer)
		vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");
	mesh->build(vertex_buffer.get());

	material->roughness = (float)roughness();
	material->metal = (float)metal();
	material->albedo = albedo();
	material->emission = emission();

	out_draw({active(), mesh_bounding_box(*mesh), [this] (const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
		draw_win(params, win, rvd);
	}});
}

void MeshRenderer::draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
	auto vb = vertex_buffer.get();
	if (!vb)
		return;

	{
		// FIXME workaround for mesh rendering bug...
		Array<vec3> points;
		points.add({0,0,0});
		points.add({0,0,0});
		session->drawing_helper->draw_lines(points, false);
	}

	session->drawing_helper->draw_mesh(params, rvd, mat4::ID, vb, material.get());
}


} // graph