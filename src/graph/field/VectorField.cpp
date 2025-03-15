//
// Created by Michael Ankele on 2025-03-14.
//

#include "VectorField.h"
#include <lib/kaba/kaba.h>
#include <lib/os/msg.h>

extern float _current_simulation_time_;

namespace graph {

void VectorField::process() {
	if (auto g = in_grid.value()) {

		if (formula() != cached_formula or !f_p) {
			cached_formula = formula();
			ctx = kaba::Context::create();

			try {
				module = ctx->create_module_for_source(format(R"foodelim(
func f(p: vec3, t: f32) -> vec3
	let x = p.x
	let y = p.y
	let z = p.z
	return %s
)foodelim", formula()));

				f_p = (f_t)module->match_function("f", "math.vec3", {"math.vec3", "f32"});

				if (!f_p)
					msg_write("not found");
			} catch (kaba::Exception& e) {
				msg_error(e.message());
				dirty = false;
				return;
			}
		}

		if (auto f = f_p) {
			artemis::data::VectorField s(*g);

			for (int i=0; i<g->nx; i++)
				for (int j=0; j<g->ny; j++)
					for (int k=0; k<g->nz; k++)
						s.set(i, j, k, vec3d(f({(float)i, (float)j, (float)k}, _current_simulation_time_)));
			out_field(s);
		}

		if (!time_dependent())
			dirty = false;
	}
}

} // graph
