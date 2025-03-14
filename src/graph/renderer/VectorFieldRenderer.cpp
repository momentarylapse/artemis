//
// Created by Michael Ankele on 2025-03-11.
//

#include "VectorFieldRenderer.h"
#include <Session.h>
#include <lib/image/color.h>
#include <view/DrawingHelper.h>

namespace graph {

void VectorFieldRenderer::draw_win(const RenderParams& params, MultiViewWindow* win) {
	auto f = in_field.value();
	if (!f)
		return;

	Array<vec3> points;
	for (int i=0; i<f->grid.nx; i++)
		for (int j=0; j<f->grid.ny; j++)
			for (int k=0; k<f->grid.nz; k++) {
				points.add(f->grid.cell_center(i, j, k));
				points.add(f->grid.cell_center(i, j, k) + f->value(i, j, k));
			}

	session->drawing_helper->set_color(_color());
	session->drawing_helper->set_line_width(line_width());
	session->drawing_helper->draw_lines(points, false);
}


} // graph