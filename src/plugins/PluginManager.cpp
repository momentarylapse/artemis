//
// Created by michi on 09.03.25.
//

#include "PluginManager.h"
#include <data/mesh/PolygonMesh.h>
#include <graph/Graph.h>
#include <graph/Node.h>
#include <lib/kaba/kaba.h>
#include <lib/math/vec2.h>
#include <lib/math/rect.h>
#include <Session.h>
#include <data/grid/RegularGrid.h>
#include <graph/NodeFactory.h>

extern Session* _current_session_;

namespace kaba {
	extern const Class* TypeVec3List;
}

namespace artemis {

base::map<const std::type_info*, const kaba::Class*> PluginManager::type_map;


class VectorField {};
class ScalarField {};

void PluginManager::init() {
	export_kaba();
	import_kaba();
}

Session* current_session() {
	return _current_session_;
}

graph::Node* graph_add_node_by_class(graph::Graph* g, const string& _class, const vec2& pos) {
	auto n = graph::create_node(g->session, _class);
	n->pos = pos;
	g->add_node(n);
	return n;
}

bool graph_connect(graph::Graph* g, graph::Node* source, int source_port, graph::Node* sink, int sink_port) {
	return g->connect(source, source_port, sink, sink_port);
}

void PluginManager::export_kaba() {
	auto ext = kaba::default_context->external.get();

	ext->link("current_session", (void*)&current_session);

	ext->declare_class_size("Mesh", sizeof(PolygonMesh));
	ext->link_class_func("Mesh.__init__", &kaba::generic_init<PolygonMesh>);

	ext->declare_class_size("ScalarField", sizeof(ScalarField));
	ext->link_class_func("ScalarField.__init__", &kaba::generic_init<ScalarField>);

	ext->declare_class_size("VectorField", sizeof(VectorField));
	ext->link_class_func("VectorField.__init__", &kaba::generic_init<VectorField>);


	ext->declare_class_size("RegularGrid", sizeof(RegularGrid));
	ext->declare_class_element("RegularGrid.nx", &RegularGrid::nx);
	ext->declare_class_element("RegularGrid.ny", &RegularGrid::ny);
	ext->declare_class_element("RegularGrid.nz", &RegularGrid::nz);
	ext->declare_class_element("RegularGrid.dx", &RegularGrid::dx);
	ext->declare_class_element("RegularGrid.dy", &RegularGrid::dy);
	ext->declare_class_element("RegularGrid.dz", &RegularGrid::dz);
	ext->link_class_func("RegularGrid.__init__", &kaba::generic_init<RegularGrid>);

	ext->declare_class_size("Session", sizeof(Session));
	ext->declare_class_element("Session.graph", &Session::graph);


	ext->declare_class_size("Node", sizeof(graph::Node));
	ext->link_class_func("Node.set", &graph::Node::set);

	ext->declare_class_size("Graph", sizeof(graph::Graph));
	ext->link_class_func("Graph.add_node", &graph_add_node_by_class);
	ext->link_class_func("Graph.connect", &graph_connect);
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
	type_map.set(&typeid(Array<vec3>), kaba::TypeVec3List);
	type_map.set(&typeid(rect), kaba::TypeRect);
	//type_map.set(&typeid(mat4), kaba::TypeMatrix4);
	//type_map.set(&typeid(box), kaba::TypeBox);
	type_map.set(&typeid(color), kaba::TypeColor);

	auto m = kaba::default_context->load_module("artemis.kaba");
	import_component_class<PolygonMesh>(m, "Mesh");
	import_component_class<RegularGrid>(m, "RegularGrid");
}

}

