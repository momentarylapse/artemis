//
// Created by Michael Ankele on 2025-03-10.
//

#include "ScalarField.h"

namespace graph {

void ScalarField::process() {
	if (auto g = in_grid.value()) {
		out_field(artemis::data::ScalarField(*g));
	}
}


} // graph