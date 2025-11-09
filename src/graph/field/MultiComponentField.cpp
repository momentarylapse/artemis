//
// Created by michi on 8/31/25.
//

#include "MultiComponentField.h"
#include <Session.h>
#include <processing/helper/GlobalThreadPool.h>
#include <lib/kaba/kaba.h>
#include <lib/os/msg.h>
#include <lib/base/iter.h>


namespace artemis {
	Session* current_session();
}

namespace artemis::graph {

	MultiComponentField::MultiComponentField() : ResourceNode("MultiComponentField") {
		time_dependent.on_update = [this] {
			if (time_dependent())
				flags = dataflow::NodeFlags::Resource | dataflow::NodeFlags::TimeDependent;
			else
				flags = dataflow::NodeFlags::Resource;
		};
	}

	void MultiComponentField::on_process() {
		if (auto g = in_grid.value()) {
			if (g->type != data::GridType::Regular)
				return;

			auto& rg = *g->regular;

			if (formula() != cached_formula or !f_p) {
				cached_formula = formula();
				ctx = kaba::Context::create();

				try {
					module = ctx->create_module_for_source(format(R"foodelim(
func f(p: vec3, t: f32) -> f32[]
	let x = p.x
	let y = p.y
	let z = p.z
	return %s
)foodelim", formula()));

					f_p = (f_t)module->match_function("f", "f32[]", {"math.vec3", "f32"});

					if (!f_p)
						msg_write("not found");
				} catch (kaba::Exception& e) {
					msg_error(e.message());
					return;
				}
			}
			float t = current_session()->t;

			if (auto f = f_p) {
				// check how many components we get...
				const auto v0 = f({0,0,0}, 0);
				int components = v0.num;

				data::MultiComponentField s(rg,
					type(),
					sampling_mode(), components);

				for (const auto& [i, p] : enumerate(g->points(sampling_mode()))) {
					const auto v = f(p, t);
					for (int n=0; n<components; n++)
						s.set(i, n, (double)v[n]);
				}

				out(s);
			}
		}
	}

} // graph

