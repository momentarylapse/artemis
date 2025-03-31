//
// Created by Michael Ankele on 2025-03-14.
//

#include "VectorField.h"
#include <lib/kaba/kaba.h>
#include <lib/os/msg.h>

namespace artemis::graph {
	extern float _current_simulation_time_;
}

namespace artemis::graph {

VectorField::VectorField() : ResourceNode("VectorField") {
	time_dependent.on_update = [this] {
		if (time_dependent())
			flags = dataflow::NodeFlags::Resource | dataflow::NodeFlags::TimeDependent;
		else
			flags = dataflow::NodeFlags::Resource;
	};
}

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
				return;
			}
		}

		if (auto f = f_p) {
			data::VectorField s(*g,
				type(),
				sampling_mode());

			switch (sampling_mode()) {
			case data::SamplingMode::PerCell:
				for (int i=0; i<g->nx; i++)
					for (int j=0; j<g->ny; j++)
						for (int k=0; k<g->nz; k++)
							s.set(i, j, k, dvec3(f({(float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f}, _current_simulation_time_)));
				break;
			case data::SamplingMode::PerVertex:
				for (int i=0; i<=g->nx; i++)
					for (int j=0; j<=g->ny; j++)
						for (int k=0; k<=g->nz; k++)
							s.set(i, j, k, dvec3(f({(float)i, (float)j, (float)k}, _current_simulation_time_)));
			}
			out(s);
		}
	}
}

} // graph
