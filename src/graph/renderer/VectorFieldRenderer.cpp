//
// Created by Michael Ankele on 2025-03-11.
//

#include "VectorFieldRenderer.h"
#include <Session.h>
#include <lib/image/color.h>
#include <view/DrawingHelper.h>

namespace artemis::graph {

void VectorFieldRenderer::on_process() {
	auto f = in_field.value();
	if (!f)
		return;

	out_draw(RenderData{active(), f->grid.bounding_box(), [this] (const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
		draw_win(params, win, rvd);
	}});
}


void VectorFieldRenderer::draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
	auto f = in_field.value();
	if (!f)
		return;

	Array<vec3> points;
	if (f->sampling_mode == artemis::data::SamplingMode::PerCell) {
		for (int i=0; i<f->grid.nx; i++)
			for (int j=0; j<f->grid.ny; j++)
				for (int k=0; k<f->grid.nz; k++) {
					points.add(f->grid.cell_center(i, j, k));
					points.add(f->grid.cell_center(i, j, k) + f->value(i, j, k).to32());
				}
	} else if (f->sampling_mode == artemis::data::SamplingMode::PerVertex) {
		for (int i=0; i<=f->grid.nx; i++)
			for (int j=0; j<=f->grid.ny; j++)
				for (int k=0; k<=f->grid.nz; k++) {
					points.add(f->grid.vertex(i, j, k));
					points.add(f->grid.vertex(i, j, k) + f->value(i, j, k).to32());
				}
	}

	session->drawing_helper->set_color(_color());
	session->drawing_helper->set_line_width((float)line_width());
	session->drawing_helper->draw_lines(points, false);
}


} // graph