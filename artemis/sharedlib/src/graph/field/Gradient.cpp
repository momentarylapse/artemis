//
// Created by Michael Ankele on 2025-03-11.
//

#include "Gradient.h"
#include <processing/field/Calculus.h>

namespace artemis::graph {

void Gradient::on_process() {
	if (auto f = in_field.value()) {
		out(artemis::processing::gradient(*f));
	}
}

} // graph