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
#include <lib/os/filesystem.h>
#include <lib/xhui/xhui.h>
#include <Session.h>
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>
#include <data/grid/RegularGrid.h>
#include <graph/NodeFactory.h>
#include <graph/Port.h>
#include <processing/field/Calculus.h>
#include <lib/os/msg.h>

extern Session* _current_session_;
extern float _current_simulation_time_;

namespace kaba {
	extern const Class* TypeVec3List;
}

namespace artemis {

base::map<const std::type_info*, const kaba::Class*> PluginManager::type_map;
base::map<string, Path> PluginManager::plugin_classes;


Path PluginManager::directory() {
	if (xhui::Application::installed)
		return xhui::Application::directory_static | "plugins";
	else
		return xhui::Application::directory_static.parent() | "plugins";
}


void PluginManager::init() {
	kaba::config.directory = directory();
	export_kaba();
	import_kaba();
	find_plugins();
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

void node_init(graph::Node* n, const string& name) {
	new(n) graph::Node(name);
}

template<class T>
void port_init(T* port, graph::Node* node, const string& name, graph::PortFlags flags) {
	new (port) T(node, name, flags);
}

template<class T>
void link_ports(kaba::ExternalLinkData* ext, const string& name) {
	ext->declare_class_size("InPort" + name, sizeof(graph::InPort<T>));
	ext->link_class_func("InPort" + name + ".__init__", &port_init<graph::InPort<T>>);
	ext->link_class_func("InPort" + name + ".value", &graph::InPort<T>::value);
	ext->link_class_func("InPort" + name + ".mutated", &graph::InPort<T>::mutated);

	ext->declare_class_size("OutPort" + name, sizeof(graph::OutPort<T>));
	ext->link_class_func("OutPort" + name + ".__init__", &port_init<graph::OutPort<T>>);
	ext->link_class_func("OutPort" + name + ".set", &graph::OutPort<T>::operator());
}

template<class T>
class GenericVDeleter : public T {
public:
	void __delete__() override {
		((T*)this)->~T();
	}

};

void PluginManager::export_kaba() {
	auto ext = kaba::default_context->external.get();

	ext->link("current_session", (void*)&current_session);
	ext->link("gradient", (void*)&processing::gradient);
	ext->link("laplace", (void*)&processing::laplace);
	ext->link("simulation_time", &_current_simulation_time_);

	ext->declare_class_size("Mesh", sizeof(PolygonMesh));
	ext->link_class_func("Mesh.__init__", &kaba::generic_init<PolygonMesh>);

	ext->declare_class_size("ScalarField", sizeof(data::ScalarField));
	ext->declare_class_element("ScalarField.grid", &data::ScalarField::grid);
	ext->declare_class_element("ScalarField.v", &data::ScalarField::v);
	ext->link_class_func("ScalarField.__init__", &kaba::generic_init<data::ScalarField>);
	ext->link_class_func("ScalarField.set", &data::ScalarField::set);
	ext->link_class_func("ScalarField.value", &data::ScalarField::value);

	ext->declare_class_size("VectorField", sizeof(data::VectorField));
	ext->link_class_func("VectorField.__init__", &kaba::generic_init<data::VectorField>);
	ext->declare_class_element("VectorField.grid", &data::VectorField::grid);
	ext->declare_class_element("VectorField.v", &data::VectorField::v);
	ext->link_class_func("VectorField.set", &data::VectorField::set);
	ext->link_class_func("VectorField.value", &data::VectorField::value);


	ext->declare_class_size("RegularGrid", sizeof(data::RegularGrid));
	ext->declare_class_element("RegularGrid.nx", &artemis::data::RegularGrid::nx);
	ext->declare_class_element("RegularGrid.ny", &artemis::data::RegularGrid::ny);
	ext->declare_class_element("RegularGrid.nz", &artemis::data::RegularGrid::nz);
	ext->declare_class_element("RegularGrid.dx", &artemis::data::RegularGrid::dx);
	ext->declare_class_element("RegularGrid.dy", &artemis::data::RegularGrid::dy);
	ext->declare_class_element("RegularGrid.dz", &artemis::data::RegularGrid::dz);
	ext->link_class_func("RegularGrid.__init__", &kaba::generic_init<artemis::data::RegularGrid>);

	ext->declare_class_size("Session", sizeof(Session));
	ext->declare_class_element("Session.graph", &Session::graph);


	{
		graph::Node n("");
		ext->declare_class_size("Node", sizeof(graph::Node));
		ext->link_class_func("Node.__init__", &node_init);
		ext->link_class_func("Node.set", &graph::Node::set);
		ext->link_virtual("Node.__delete__", &GenericVDeleter<graph::Node>::__delete__, &n);
		ext->link_virtual("Node.process", &graph::Node::process, &n);
		ext->link_virtual("Node.create_panel", &graph::Node::create_panel, &n);
	}

	link_ports<artemis::data::ScalarField>(ext, "ScalarField");
	link_ports<artemis::data::VectorField>(ext, "VectorField");

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

	auto m = kaba::default_context->load_module("artemis/artemis.kaba");
	import_component_class<PolygonMesh>(m, "Mesh");
	import_component_class<artemis::data::RegularGrid>(m, "RegularGrid");
	import_component_class<artemis::data::ScalarField>(m, "ScalarField");
	import_component_class<artemis::data::VectorField>(m, "VectorField");
}

void PluginManager::find_plugins() {
	Path dir = directory() | "simulation";
	for (const auto& e: os::fs::search(dir, "*.kaba", "f")) {
		string name = e.basename_no_ext();
		plugin_classes.set(name, dir | e);
	}
}

void* PluginManager::create_instance(const string& name) {
	for (const auto& [n, f] : artemis::PluginManager::plugin_classes)
		if (name == n) {
			auto m = kaba::default_context->load_module(f);
			for (const auto c: m->classes())
				if (c->name == name)
					return c->create_instance();
		}
	return nullptr;
}



}

