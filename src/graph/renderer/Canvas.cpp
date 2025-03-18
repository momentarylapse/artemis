//
// Created by Michael Ankele on 2025-03-17.
//

#include "Canvas.h"

namespace graph {
void Canvas::draw_win(const RenderParams& params, MultiViewWindow* win) {

}

void Canvas::draw_2d(Painter* p) {
	for (auto& d: in_draw.values()) {
		if (d->f_draw_2d)
			d->f_draw_2d(p);
	}
}


} // graph