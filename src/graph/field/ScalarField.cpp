//
// Created by Michael Ankele on 2025-03-10.
//

#include "ScalarField.h"
#include "../Graph.h"
#include <lib/kaba/kaba.h>
#include <lib/os/msg.h>
#include <lib/base/iter.h>

namespace artemis::graph {

ScalarField::ScalarField() : ResourceNode("ScalarField") {
	time_dependent.on_update = [this] {
		if (time_dependent())
			flags = dataflow::NodeFlags::Resource | dataflow::NodeFlags::TimeDependent;
		else
			flags = dataflow::NodeFlags::Resource;
	};
}

void ScalarField::on_process() {
	if (auto g = in_grid.value()) {
		if (g->type != data::GridType::Regular)
			return;
		auto& rg = *g->regular;

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

		float t = static_cast<Graph*>(graph)->t;

		if (auto f = f_p) {
			data::ScalarField s(rg,
				type(),
				sampling_mode());

			for (const auto& [i, p] : enumerate(g->points(sampling_mode())))
				s._set(i, (double)f(p, t));

			out(s);
		}
	}
}

} // graph
