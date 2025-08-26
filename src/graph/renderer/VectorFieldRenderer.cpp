//
// Created by Michael Ankele on 2025-03-11.
//

#include "VectorFieldRenderer.h"
#include <Session.h>
#include <lib/image/color.h>
#include <lib/base/iter.h>
#include <view/DrawingHelper.h>

namespace artemis::graph {

void VectorFieldRenderer::on_process() {
	send_out();
}

base::optional<Box> VectorFieldRenderer::bounding_box() const {
	return in_field.value()->grid.bounding_box();
}

void VectorFieldRenderer::on_emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) {
	auto f = in_field.value();
	if (!f)
		return;

	float s = (float)scale();

	Array<vec3> points;
	for (const auto& [i, p]: enumerate(f->grid.points(f->sampling_mode))) {
		points.add(p);
		points.add(p + f->value32(i) * s);
	}

	session->drawing_helper->set_color(_color());
	session->drawing_helper->set_line_width((float)line_width());
	session->drawing_helper->draw_lines(points, false);
}


} // graph