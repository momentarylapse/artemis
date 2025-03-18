//
// Created by Michael Ankele on 2025-03-17.
//

#include "FunctionPlot.h"
#include <lib/os/msg.h>

namespace graph {

void FunctionPlot::process() {
	if (formula() != cached_formula or !f_p) {
		cached_formula = formula();
		ctx = kaba::Context::create();

		try {
			module = ctx->create_module_for_source(format(R"foodelim(
func f(x: f32) -> f32
	return %s
)foodelim", formula()));

			f_p = (f_t)module->match_function("f", "f32", {"f32"});

			if (!f_p)
				msg_write("not found");
		} catch (kaba::Exception& e) {
			msg_error(e.message());
			return;
		}
	}

	if (auto f = f_p)
		out_plot({line_width(), _color(), f_p});
}
} // graph