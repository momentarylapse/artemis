//
// Created by michi on 7/26/25.
//

#include "ListToVectors.h"
#include <lib/math/vec3.h>

namespace artemis::graph {

	void ListToVectors::on_process() {
		if (!out_vectors.value.has_value())
			out_vectors.value = Array<vec3>{};

		if (first() < 0)
			return;
		if (stride() <= 0)
			return;

		if (in_numbers.has_value()) {
			for (int i=first(); i<in_numbers.value()->num-2; i+=stride())
				out_vectors.value->add(vec3((float)(*in_numbers.value())[i], (float)(*in_numbers.value())[i+1], (float)(*in_numbers.value())[i+2]));
			out_vectors.mutated();
		}
	}


} // graph