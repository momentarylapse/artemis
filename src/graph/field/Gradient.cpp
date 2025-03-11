//
// Created by Michael Ankele on 2025-03-11.
//

#include "Gradient.h"
#include <processing/field/Gradient.h>

namespace graph {

void Gradient::process() {
	if (auto f = in_field.value()) {
		out(artemis::processing::gradient(*f));
		dirty = false;
	}
}

} // graph