//
// Created by michi on 09.03.25.
//

#include "PluginManager.h"
#include <data/mesh/PolygonMesh.h>
#include <lib/kaba/kaba.h>
#include <lib/math/vec2.h>
#include <lib/math/rect.h>

namespace artemis {

base::map<const std::type_info*, const kaba::Class*> PluginManager::type_map;


class VectorField {};
class ScalarField {};

void PluginManager::init() {
	export_kaba();
	import_kaba();
}

void PluginManager::export_kaba() {
	auto ext = kaba::default_context->external.get();

	ext->declare_class_size("Mesh", sizeof(PolygonMesh));
	ext->link_class_func("Mesh.__init__", &kaba::generic_init<PolygonMesh>);

	ext->declare_class_size("ScalarField", sizeof(ScalarField));
	ext->link_class_func("ScalarField.__init__", &kaba::generic_init<ScalarField>);

	ext->declare_class_size("VectorField", sizeof(VectorField));
	ext->link_class_func("VectorField.__init__", &kaba::generic_init<VectorField>);
}

template<class C>
void import_component_class(shared<kaba::Module> m, const string &name) {
	for (auto c: m->classes())
		if (c->name == name) {
			PluginManager::type_map.set(&typeid(C), c);
			return;
		}
	throw Exception(format("artemis.kaba: %s missing", name));
}

void PluginManager::import_kaba() {
	type_map.set(&typeid(float), kaba::TypeFloat32);
	type_map.set(&typeid(double), kaba::TypeFloat64);
	type_map.set(&typeid(int), kaba::TypeInt32);
	type_map.set(&typeid(int64), kaba::TypeInt64);
	type_map.set(&typeid(bool), kaba::TypeBool);
	type_map.set(&typeid(string), kaba::TypeString);
	type_map.set(&typeid(vec2), kaba::TypeVec2);
	type_map.set(&typeid(vec3), kaba::TypeVec3);
	type_map.set(&typeid(rect), kaba::TypeRect);
	//type_map.set(&typeid(mat4), kaba::TypeMatrix4);
	//type_map.set(&typeid(box), kaba::TypeBox);
	type_map.set(&typeid(color), kaba::TypeColor);

	auto m = kaba::default_context->load_module("artemis.kaba");
	import_component_class<PolygonMesh>(m, "Mesh");
}

}

