//
// Created by michi on 8/16/25.
//

#include "VectorFieldComponent.h"

namespace artemis::graph {

void VectorFieldComponent::on_process() {
	int n = component();
	if (n < 0 or n >= 3)
		return;
	if (auto f = in_field.value()) {
		out(f->get_component(n));
	}
}

} // graph