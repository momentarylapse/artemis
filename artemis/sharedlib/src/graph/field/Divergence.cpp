//
// Created by Michael Ankele on 2025-03-14.
//

#include "Divergence.h"
#include <processing/field/Calculus.h>

namespace artemis::graph {

void Divergence::on_process() {
	if (auto f = in_field.value()) {
		out(artemis::processing::divergence(*f));
	}
}

} // graph