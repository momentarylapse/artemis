//
// Created by michi on 7/26/25.
//

#include "NumberList.h"

namespace artemis::graph {

NumberList::NumberList() : ResourceNode("NumberList") {
	flags = dataflow::NodeFlags::Resource;
}

void NumberList::on_process() {
	out(list());
}

} // graph

