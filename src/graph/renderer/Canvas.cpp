//
// Created by Michael Ankele on 2025-03-17.
//

#include "Canvas.h"
#include <Session.h>
#include <view/DrawingHelper.h>
#include <view/MultiView.h>

namespace graph {

void Canvas::draw_win(const RenderParams& params, MultiViewWindow* win) {
	session->drawing_helper->clear(params, background());

	if (!camera_defined) {
		base::optional<Box> bounding_box;
		for (auto& d: in_draw.values()) {
			if (d->bounding_box) {
				if (bounding_box)
					bounding_box = *bounding_box or *d->bounding_box;
				else
					bounding_box = *d->bounding_box;
			}
		}
		if (bounding_box) {
			win->multi_view->view_port.pos = bounding_box->center();
			win->multi_view->view_port.radius = bounding_box->size().length() * 1.2f;
			camera_defined = true;
		}
	}
}

void Canvas::draw_2d(Painter* p) {
	for (auto& d: in_draw.values()) {
		if (d->f_draw_2d)
			d->f_draw_2d(p);
	}
}


} // graph