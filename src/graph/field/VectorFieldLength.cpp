//
// Created by michi on 8/16/25.
//

#include "VectorFieldLength.h"

namespace artemis::graph {

void VectorFieldLength::on_process() {
	if (auto f = in_field.value()) {
		out(f->length());
	}
}

} // graph
