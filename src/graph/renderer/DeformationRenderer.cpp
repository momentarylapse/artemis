#include "DeformationRenderer.h"
#include <Session.h>
#include <data/mesh/GeometrySphere.h>
#include <view/DrawingHelper.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/base/iter.h>

namespace artemis::graph {

DeformationRenderer::DeformationRenderer(Session* s) : RendererNode(s, "DeformationRenderer") {}

void DeformationRenderer::on_process() {
	auto d = in_diff.value();
	if (!d)
		return;

	out_draw(RenderData{active(), d->grid.bounding_box(), [this] (const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
		draw_win(params, win, rvd);
	}});
}


void DeformationRenderer::draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {

	auto d = in_diff.value();
	if (!d)
		return;

	if (!vertex_buffer)
		vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");

	GeometrySphere mesh(v_0, 1, 2);
	mesh.build(vertex_buffer.get());

	float s = (float)scale();
	float r = (float)radius();
	const auto& g = d->grid;

	if (show_balls()) {
		for (const auto& [i, p]: enumerate(g.points(d->sampling_mode))) {
			vec3 pos = p + d->value32(i) * s;
			session->drawing_helper->draw_mesh(params, rvd,
				mat4::translation(pos) * mat4::scale(r, r, r),
				vertex_buffer.get(),
				session->drawing_helper->material_selection);
		}
	}

	if (d->sampling_mode == artemis::data::SamplingMode::PerCell) {

		if (show_grid()) {
			Array<vec3> points;
			for (int i=0; i<g.nx; i++)
				for (int j=0; j<g.ny; j++)
					for (int k=0; k<g.nz-1; k++) {
						points.add(g.cell_center(i, j, k) + d->_value32(i, j, k) * s);
						points.add(g.cell_center(i, j, k + 1) + d->_value32(i, j, k + 1) * s);
					}
			for (int i=0; i<g.nx; i++)
				for (int j=0; j<g.ny-1; j++)
					for (int k=0; k<g.nz; k++) {
						points.add(g.cell_center(i, j, k) + d->_value32(i, j, k) * s);
						points.add(g.cell_center(i, j + 1, k) + d->_value32(i, j + 1, k) * s);
					}
			for (int i=0; i<g.nx-1; i++)
				for (int j=0; j<g.ny; j++)
					for (int k=0; k<g.nz; k++) {
						points.add(g.cell_center(i, j, k) + d->_value32(i, j, k) * s);
						points.add(g.cell_center(i + 1, j, k) + d->_value32(i + 1, j, k) * s);
					}

			session->drawing_helper->set_color(_color());
			session->drawing_helper->set_line_width((float)line_width());
			session->drawing_helper->draw_lines(points, false);
		}
	} else if (d->sampling_mode == artemis::data::SamplingMode::PerVertex) {

		if (show_grid()) {
			Array<vec3> points;
			auto p0 = g.vertices();
			for (const auto& [a, b]: g.edges()) {
				points.add(p0[a] + d->value(a).to32() * s);
				points.add(p0[b] + d->value(b).to32() * s);
			}

			session->drawing_helper->set_color(_color());
			session->drawing_helper->set_line_width((float)line_width());
			session->drawing_helper->draw_lines(points, false);
		}
	}
}


} // graph
