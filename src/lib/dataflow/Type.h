//
// Created by Michael Ankele on 2025-03-31.
//

#ifndef TYPE_H
#define TYPE_H

#include <typeinfo>
#include <lib/base/base.h>
#include <lib/base/map.h>

namespace kaba {
	class Class;
}

namespace dataflow {

extern base::map<const std::type_info*, const kaba::Class*> type_map;

void link_basic_types();

template<class T>
const kaba::Class* get_class() {
	return type_map[&typeid(T)];
}

} // dataflow

#endif //TYPE_H
