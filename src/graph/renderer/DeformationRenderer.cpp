#include "DeformationRenderer.h"
#include <Session.h>
#include <data/mesh/GeometrySphere.h>
#include <view/DrawingHelper.h>
#include <lib/ygraphics/graphics-impl.h>

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

	if (d->sampling_mode == artemis::data::SamplingMode::PerCell) {
		if (show_balls()) {
			for (int i=0; i<g.nx; i++)
				for (int j=0; j<g.ny; j++)
					for (int k=0; k<g.nz; k++) {
						vec3 pos = g.cell_center(i, j, k) + d->value32(i, j, k) * s;
						session->drawing_helper->draw_mesh(params, rvd,
							mat4::translation(pos) * mat4::scale(r, r, r),
							vertex_buffer.get(),
							session->drawing_helper->material_selection);
					}
		}

		if (show_grid()) {
			Array<vec3> points;
			for (int i=0; i<g.nx; i++)
				for (int j=0; j<g.ny; j++)
					for (int k=0; k<g.nz-1; k++) {
						points.add(g.cell_center(i, j, k) + d->value32(i, j, k) * s);
						points.add(g.cell_center(i, j, k + 1) + d->value32(i, j, k + 1) * s);
					}
			for (int i=0; i<g.nx; i++)
				for (int j=0; j<g.ny-1; j++)
					for (int k=0; k<g.nz; k++) {
						points.add(g.cell_center(i, j, k) + d->value32(i, j, k) * s);
						points.add(g.cell_center(i, j + 1, k) + d->value32(i, j + 1, k) * s);
					}
			for (int i=0; i<g.nx-1; i++)
				for (int j=0; j<g.ny; j++)
					for (int k=0; k<g.nz; k++) {
						points.add(g.cell_center(i, j, k) + d->value32(i, j, k) * s);
						points.add(g.cell_center(i + 1, j, k) + d->value32(i + 1, j, k) * s);
					}

			session->drawing_helper->set_color(_color());
			session->drawing_helper->set_line_width((float)line_width());
			session->drawing_helper->draw_lines(points, false);
		}
	} else if (d->sampling_mode == artemis::data::SamplingMode::PerVertex) {
		if (show_balls()) {
			for (int i=0; i<=g.nx; i++)
				for (int j=0; j<=g.ny; j++)
					for (int k=0; k<=g.nz; k++) {
						vec3 pos = g.vertex(i, j, k) + d->value32(i, j, k) * s;
						session->drawing_helper->draw_mesh(params, rvd,
							mat4::translation(pos) * mat4::scale(r, r, r),
							vertex_buffer.get(),
							session->drawing_helper->material_selection);
					}
		}

		if (show_grid()) {
			Array<vec3> points;
			for (int i=0; i<=g.nx; i++)
				for (int j=0; j<=g.ny; j++)
					for (int k=0; k<g.nz; k++) {
						points.add(g.vertex(i, j, k) + d->value32(i, j, k) * s);
						points.add(g.vertex(i, j, k + 1) + d->value32(i, j, k + 1) * s);
					}
			for (int i=0; i<=g.nx; i++)
				for (int j=0; j<g.ny; j++)
					for (int k=0; k<=g.nz; k++) {
						points.add(g.vertex(i, j, k) + d->value32(i, j, k) * s);
						points.add(g.vertex(i, j + 1, k) + d->value32(i, j + 1, k) * s);
					}
			for (int i=0; i<g.nx; i++)
				for (int j=0; j<=g.ny; j++)
					for (int k=0; k<=g.nz; k++) {
						points.add(g.vertex(i, j, k) + d->value32(i, j, k) * s);
						points.add(g.vertex(i + 1, j, k) + d->value32(i + 1, j, k) * s);
					}

			session->drawing_helper->set_color(_color());
			session->drawing_helper->set_line_width((float)line_width());
			session->drawing_helper->draw_lines(points, false);
		}
	}
}


} // graph
