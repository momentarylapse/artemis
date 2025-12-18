//
// Created by Michael Ankele on 2025-03-31.
//

#include "Type.h"
#include <lib/kaba/kaba.h>
#include <lib/math/vec2.h>
#include <lib/math/vec3.h>
#include <lib/math/rect.h>
#include <lib/image/color.h>


namespace dataflow {

base::map<const std::type_info*, const kaba::Class*> type_map;

void link_basic_types() {
	type_map.set(&typeid(float), kaba::common_types.f32);
	type_map.set(&typeid(Array<float>), kaba::common_types.f32_list);
	type_map.set(&typeid(double), kaba::common_types.f64);
	type_map.set(&typeid(Array<double>), kaba::common_types.f64_list);
	type_map.set(&typeid(int), kaba::common_types.i32);
	type_map.set(&typeid(Array<int>), kaba::common_types.i32_list);
	type_map.set(&typeid(int64), kaba::common_types.i64);
	type_map.set(&typeid(bool), kaba::common_types._bool);
	type_map.set(&typeid(string), kaba::common_types.string);
	type_map.set(&typeid(vec2), kaba::common_types.vec2);
	type_map.set(&typeid(vec3), kaba::common_types.vec3);
	type_map.set(&typeid(Array<vec3>), kaba::common_types.vec3_list);
	type_map.set(&typeid(rect), kaba::common_types.rect);
	//type_map.set(&typeid(mat4), kaba::common_types.mat4);
	//type_map.set(&typeid(box), kaba::common_types.box);
	type_map.set(&typeid(color), kaba::common_types.color);
}


const kaba::Class* _get_class(const std::type_info* t) {
	try {
		return type_map[t];
	} catch (Exception& e) {
		throw Exception(format("type not registered: %s", t->name()));
	}
}

} // dataflow