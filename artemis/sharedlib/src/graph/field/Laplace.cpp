//
// Created by Michael Ankele on 2025-03-13.
//

#include "Laplace.h"
#include <processing/field/Calculus.h>

namespace artemis::graph {

void Laplace::on_process() {
	if (auto f = in_field.value()) {
		out(artemis::processing::laplace(*f));
	}
}

} // graph