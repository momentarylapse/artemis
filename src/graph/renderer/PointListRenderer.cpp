//
// Created by Michael Ankele on 2025-03-10.
//

#include "PointListRenderer.h"
#include <Session.h>
#include <data/mesh/GeometrySphere.h>
#include <view/DrawingHelper.h>
#include <y/renderer/base.h>

namespace artemis::graph {

base::optional<Box> point_list_bounding_box(const Array<vec3>& points) {
	if (points.num == 0)
		return base::None;
	Box b = {points[0], points[0]};
	for (const vec3& p: points)
		b = b or Box{p, p};
	return b;
}

PointListRenderer::PointListRenderer(Session* s) : RendererNode(s, "PointListRenderer") {
	material = new Material(s->resource_manager);
	material->textures.add(tex_white);
}

void PointListRenderer::on_process() {
	if (!active())
		return;

	if (auto points = in_points.value()) {
		// TODO
		out_draw(RenderData{active(), point_list_bounding_box(*points), [this] (const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd) {
			draw_win(params, win, rvd);
		}});
	}
}


void PointListRenderer::draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd) {
	if (!vertex_buffer)
		vertex_buffer = new VertexBuffer("3f,3f,2f");

	// mesh -> vb
	if (in_mesh.value()) {
		in_mesh.value()->build(vertex_buffer.get());
	} else {
		GeometrySphere mesh(v_0, (float)radius(), 4);
		mesh.build(vertex_buffer.get());
	}

	material->albedo = Black;
	material->emission = _color();

	if (auto points = in_points.value()) {
		//session->drawing_helper->draw_data_points()
		for (const auto& p: *points)
			session->drawing_helper->draw_mesh(params, rvd, mat4::translation(p), vertex_buffer.get(), material.get());
	}
}


} // graph