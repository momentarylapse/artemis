//
// Created by Michael Ankele on 2025-03-31.
//

#pragma once

#include <typeinfo>
#include <lib/base/base.h>
#include <lib/base/map.h>

namespace kaba {
	class Class;
}

namespace dataflow {

extern const kaba::Class* generic_type;

extern base::map<const std::type_info*, const kaba::Class*> type_map;

void link_basic_types();
const kaba::Class* _get_class(const std::type_info* t);

template<class T>
const kaba::Class* get_class() {
	return _get_class(&typeid(T));
}

struct GenericData {};

} // dataflow

