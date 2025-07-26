//
// Created by Michael Ankele on 2025-03-18.
//

#include "NumberListAccumulator.h"

namespace artemis::graph {

void NumberListAccumulator::on_process() {
	if (!out_list.value.has_value())
		out_list.value = Array<double>{};

	if (in_number.has_value()) {
		out_list.value->add(*in_number.value());
		if (capacity() > 0)
			while (out_list.value->num > capacity())
				out_list.value->erase(0);

		out_list.mutated();
	}
}


} // graph