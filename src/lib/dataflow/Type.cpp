//
// Created by Michael Ankele on 2025-03-31.
//

#include "Type.h"
#include <lib/kaba/kaba.h>
#include <lib/math/vec2.h>
#include <lib/math/vec3.h>
#include <lib/math/rect.h>
#include <lib/image/color.h>

namespace kaba {
	extern const Class* TypeVec3List;
	extern const Class* TypeFloatList;
}


namespace dataflow {

base::map<const std::type_info*, const kaba::Class*> type_map;

void link_basic_types() {
	type_map.set(&typeid(float), kaba::TypeFloat32);
	type_map.set(&typeid(Array<float>), kaba::TypeFloatList);
	type_map.set(&typeid(double), kaba::TypeFloat64);
	type_map.set(&typeid(int), kaba::TypeInt32);
	type_map.set(&typeid(int64), kaba::TypeInt64);
	type_map.set(&typeid(bool), kaba::TypeBool);
	type_map.set(&typeid(string), kaba::TypeString);
	type_map.set(&typeid(vec2), kaba::TypeVec2);
	type_map.set(&typeid(vec3), kaba::TypeVec3);
	type_map.set(&typeid(Array<vec3>), kaba::TypeVec3List);
	type_map.set(&typeid(rect), kaba::TypeRect);
	//type_map.set(&typeid(mat4), kaba::TypeMatrix4);
	//type_map.set(&typeid(box), kaba::TypeBox);
	type_map.set(&typeid(color), kaba::TypeColor);
}

} // dataflow