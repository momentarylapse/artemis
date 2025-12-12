//
// Created by Michael Ankele on 2025-03-14.
//

#include "Rotation.h"
#include <processing/field/Calculus.h>

namespace artemis::graph {

void Rotation::on_process() {
	if (auto f = in_field.value()) {
		if (backwards())
			out(artemis::processing::rotation_bw(*f));
		else
			out(artemis::processing::rotation_fw(*f));
	}
}

} // graph