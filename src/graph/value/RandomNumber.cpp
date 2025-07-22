//
// Created by Michael Ankele on 2025-03-18.
//

#include "RandomNumber.h"

namespace artemis::graph {

void RandomNumber::on_process() {
	out_number(randf(1.0f));
}

} // graph