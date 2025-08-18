//
// Created by Michael Ankele on 2025-03-18.
//

#include "ListAccumulator.h"

namespace artemis::graph {

void ListAccumulator::on_process() {
	if (in_number.has_value()) {
		out_list.value().add(*in_number.value());
		if (capacity() > 0)
			while (out_list.value().num > capacity())
				out_list.value().erase(0);

		out_list.mutated();
	}
}


} // graph