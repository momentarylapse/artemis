//
// Created by Michael Ankele on 2025-03-17.
//

#include "FunctionPlot.h"
#include <lib/os/msg.h>
#include <cmath>

namespace artemis::graph {

void FunctionPlot::on_process() {
	if (formula() != cached_formula or !f_p) {
		cached_formula = formula();
		ctx = kaba::default_context->create_new_context();
		f_p = nullptr;

		try {
			module = ctx->create_module_for_source(format(R"foodelim(
func f(x: f32) -> f32
	return %s
)foodelim", formula()), "<temp>", false);

			f_p = (f_t)module->match_function("f", "f32", {"f32"});

			if (!f_p)
				msg_write("not found");
		} catch (kaba::Exception& e) {
			msg_error(e.message());
			return;
		}
	}

	out_plot({this, PlotMode::Function});
}

float FunctionPlot::plot_function(float x) {
	if (f_p)
		return (*f_p)(x);
	return NAN;
}
} // graph