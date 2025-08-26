//
// Created by michi on 09.03.25.
//

#include "MeshRenderer.h"
#include <Session.h>
#include <view/DrawingHelper.h>
#include <view/MultiView.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/Context.h>

#include "lib/os/msg.h"

namespace artemis::graph {

base::optional<Box> mesh_bounding_box(const PolygonMesh& mesh) {
	if (mesh.vertices.num == 0)
		return base::None;
	Box b = {mesh.vertices[0].pos, mesh.vertices[0].pos};
	for (const auto& v: mesh.vertices)
		b = b or Box{v.pos, v.pos};
	return b;
}

MeshRenderer::MeshRenderer(Session* s) : RenderEmitterNode(s, "MeshRenderer") {
	if (!s->ctx)
		msg_error("no ctx");
	material = new yrenderer::Material(s->ctx);
	material->textures.add(s->ctx->tex_white);
	vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");
}

MeshRenderer::~MeshRenderer() = default;

void MeshRenderer::on_process() {
	auto mesh = in_mesh.value();
	if (!mesh)
		return;

	mesh->build(vertex_buffer.get());

	material->roughness = (float)roughness();
	material->metal = (float)metal();
	material->albedo = albedo();
	material->emission = emission();

	send_out();
}

void MeshRenderer::on_emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) {
	auto vb = vertex_buffer.get();
	session->drawing_helper->draw_mesh(params, rvd, mat4::ID, vb, material.get());
}

base::optional<Box> MeshRenderer::bounding_box() const {
	return mesh_bounding_box(*in_mesh.value());
}



} // graph