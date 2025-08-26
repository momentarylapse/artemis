//
// Created by Michael Ankele on 2025-03-17.
//

#include "Canvas.h"
#include "view/Canvas.h"

#include <Session.h>
#include <view/DrawingHelper.h>
#include <view/MultiView.h>
#include "view/ArtemisWindow.h"

namespace artemis::graph {

void Canvas::on_process() {
	view::RenderNode* child = nullptr;
	if (in_draw.has_value())
		child = in_draw.value()->render_node;
	session->win->canvas->set_child(child);
}

/*
void Canvas::draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
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
			win->multi_view->view_port.radius = bounding_box->size().length() * 1.5f;
			camera_defined = true;
		}
	}

	for (auto d: in_draw.values())
		if (d->f_draw_3d and d->active)
			d->f_draw_3d(params, win, rvd);

	for (auto d: in_draw.values())
		if (d->f_draw_3d_transparent and d->active)
			d->f_draw_3d_transparent(params, win, rvd);
}*/


} // graph