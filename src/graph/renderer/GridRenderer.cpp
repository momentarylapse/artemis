//
// Created by Michael Ankele on 2025-03-10.
//

#include "GridRenderer.h"

#include <Session.h>
#include <lib/os/msg.h>
#include <view/DrawingHelper.h>

namespace artemis::graph {

void GridRenderer::on_process() {
	if (auto r = regular.value()) {
		out_draw(RenderData{active(), r->bounding_box(), [this] (const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
			draw_win(params, win, rvd);
		}});
	}
}


void GridRenderer::draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
	if (auto g = regular.value()) {
		Array<vec3> points;
		if (g->type == data::GridType::Regular) {
			auto r = *g->regular;
			if (mode() == RenderMode::Outlines) {
				for (int i=0; i<=r.nx; i+=r.nx)
					for (int j=0; j<=r.ny; j+=r.ny) {
						points.add(r.vertex(i, j, 0));
						points.add(r.vertex(i, j, r.nz));
					}
				for (int i=0; i<=r.nx; i+=r.nx)
					for (int k=0; k<=r.nz; k+=r.nz) {
						points.add(r.vertex(i, 0, k));
						points.add(r.vertex(i, r.ny, k));
					}
				for (int j=0; j<=r.ny; j+=r.ny)
					for (int k=0; k<=r.nz; k+=r.nz) {
						points.add(r.vertex(0, j, k));
						points.add(r.vertex(r.nx, j, k));
					}
			} else {
				for (int i=0; i<=r.nx; i++)
					for (int j=0; j<=r.ny; j++) {
						points.add(r.vertex(i, j, 0));
						points.add(r.vertex(i, j, r.nz));
					}
				for (int i=0; i<=r.nx; i++)
					for (int k=0; k<=r.nz; k++) {
						points.add(r.vertex(i, 0, k));
						points.add(r.vertex(i, r.ny, k));
					}
				for (int j=0; j<=r.ny; j++)
					for (int k=0; k<=r.nz; k++) {
						points.add(r.vertex(0, j, k));
						points.add(r.vertex(r.nx, j, k));
					}
			}
		} else {
			// if (mode() == RenderMode::Outlines) {} TODO
			const auto vertices = g->vertices();
			for (const auto& [a, b]: g->edges()) {
				points.add(vertices[a]);
				points.add(vertices[b]);
			}
		}

		session->drawing_helper->set_color(_color());
		session->drawing_helper->set_line_width((float)line_width());
		session->drawing_helper->draw_lines(points, false);
	}
}

} // graph