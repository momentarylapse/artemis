//
// Created by Michael Ankele on 2025-03-18.
//

#include "ListPlot.h"
#include <lib/base/iter.h>

namespace artemis::graph {

void ListPlot::on_process() {
	Array<vec2> points;
	if (in_list.has_value()) {
		for (const auto& [i, y]: enumerate(*in_list.value()))
			points.add(vec2((float)i * (float)x_scale(), (float)y));
		out_plot({line_width(), _color(), nullptr, points});
	}
}
} // graph