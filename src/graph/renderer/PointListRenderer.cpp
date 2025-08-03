//
// Created by Michael Ankele on 2025-03-10.
//

#include "PointListRenderer.h"
#include <Session.h>
#include <data/mesh/GeometrySphere.h>
#include <view/DrawingHelper.h>
#include <lib/yrenderer/base.h>

#include "lib/base/iter.h"

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
	material = new yrenderer::Material(s->ctx);
	material->textures.add(s->ctx->tex_white);
}

void PointListRenderer::on_process() {
	if (trail_length() > 0) {
		trails.resize(in_points.value()->num);
		for (const auto& [i, v]: enumerate(*in_points.value())) {
			trails[i].add(v);
			while (trails[i].num > trail_length())
				trails[i].erase(0);
		}
	} else {
		trails.clear();
	}

	if (!active())
		return;

	if (auto points = in_points.value()) {
		// TODO
		out_draw(RenderData{active(), point_list_bounding_box(*points), [this] (const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
			draw_win(params, win, rvd);
		}});
	}
}


void PointListRenderer::draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
	if (!vertex_buffer)
		vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");

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

	if (trail_length() > 0) {
		for (const auto& points: trails) {
			session->drawing_helper->set_color(_color());
			session->drawing_helper->set_line_width((float)line_width());
			session->drawing_helper->draw_lines(points, true);
		}
	}
}


} // graph