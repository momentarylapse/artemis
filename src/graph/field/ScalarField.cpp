//
// Created by Michael Ankele on 2025-03-10.
//

#include "ScalarField.h"
#include <lib/kaba/kaba.h>
#include <lib/os/msg.h>

extern float _current_simulation_time_;

namespace graph {

ScalarField::ScalarField() : ResourceNode("ScalarField") {
	time_dependent.on_update = [this] {
		if (time_dependent())
			flags = NodeFlags::Resource | NodeFlags::TimeDependent;
		else
			flags = NodeFlags::Resource;
	};
}

void ScalarField::process() {
	if (auto g = in_grid.value()) {

		if (formula() != cached_formula or !f_p) {
			cached_formula = formula();
			ctx = kaba::Context::create();

			try {
				module = ctx->create_module_for_source(format(R"foodelim(
func f(p: vec3, t: f32) -> f32
	let x = p.x
	let y = p.y
	let z = p.z
	return %s
)foodelim", formula()));

				f_p = (f_t)module->match_function("f", "f32", {"math.vec3", "f32"});

				if (!f_p)
					msg_write("not found");
			} catch (kaba::Exception& e) {
				msg_error(e.message());
				return;
			}
		}

		if (auto f = f_p) {
			artemis::data::ScalarField s(*g);

			for (int i=0; i<g->nx; i++)
				for (int j=0; j<g->ny; j++)
					for (int k=0; k<g->nz; k++)
						s.set(i, j, k, f({(float)i, (float)j, (float)k}, _current_simulation_time_));
			out_field(s);
		}
	}
}

} // graph
