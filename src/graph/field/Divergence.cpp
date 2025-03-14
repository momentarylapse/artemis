//
// Created by Michael Ankele on 2025-03-14.
//

#include "Divergence.h"
#include <processing/field/Calculus.h>

namespace graph {

void Divergence::process() {
	if (auto f = in_field.value()) {
		out(artemis::processing::divergence(*f));
		dirty = false;
	}
}

} // graph