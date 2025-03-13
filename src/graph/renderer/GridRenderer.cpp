//
// Created by Michael Ankele on 2025-03-10.
//

#include "GridRenderer.h"

#include <Session.h>
#include <lib/os/msg.h>
#include <view/DrawingHelper.h>

namespace graph {

void GridRenderer::draw_win(const RenderParams& params, MultiViewWindow* win) {
	if (auto r = regular.value()) {
		Array<vec3> points;
		for (int i=0; i<r->nx; i++)
			for (int j=0; j<r->ny; j++) {
				points.add(r->index_to_pos(i, j, 0));
				points.add(r->index_to_pos(i, j, r->nz-1));
			}
		for (int i=0; i<r->nx; i++)
			for (int k=0; k<r->nz; k++) {
				points.add(r->index_to_pos(i, 0, k));
				points.add(r->index_to_pos(i, r->ny-1, k));
			}
		for (int j=0; j<r->ny; j++)
			for (int k=0; k<r->nz; k++) {
				points.add(r->index_to_pos(0, j, k));
				points.add(r->index_to_pos(r->nx-1, j, k));
			}

		session->drawing_helper->set_color(_color());
		session->drawing_helper->set_line_width(line_width());
		session->drawing_helper->draw_lines(points, false);
	}
}

} // graph