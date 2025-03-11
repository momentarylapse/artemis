//
// Created by Michael Ankele on 2025-03-10.
//

#include "ScalarField.h"
#include <lib/kaba/kaba.h>
#include <lib/os/msg.h>

namespace graph {

void ScalarField::process() {
	if (auto g = in_grid.value()) {
		auto ctx = kaba::Context::create();

		try {
			auto m = ctx->create_module_for_source(format(R"foodelim(
func f(p: vec3) -> f32
	let x = p.x
	let y = p.y
	let z = p.z
	return %s
)foodelim", formula()));

			typedef float (*f_p)(const vec3&);

			if (auto f = (f_p)m->match_function("f", "f32", {"math.vec3"})) {
				artemis::data::ScalarField s(*g);

				for (int i=0; i<g->nx; i++)
					for (int j=0; j<g->ny; j++)
						for (int k=0; k<g->nz; k++)
							s.set(i, j, k, f({(float)i, (float)j, (float)k}));
				out_field(s);
			} else {
				msg_write("not found");
				out_field(artemis::data::ScalarField(*g));
			}
		} catch (kaba::Exception& e) {
			msg_error(e.message());
		}

		dirty = false;
	}
}


} // graph