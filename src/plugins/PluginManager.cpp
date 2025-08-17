//
// Created by michi on 09.03.25.
//

#include "PluginManager.h"
#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Type.h>
#include <lib/kaba/kaba.h>
#include <lib/math/vec2.h>
#include <lib/math/rect.h>
#include <lib/os/app.h>
#include <lib/os/filesystem.h>
#include <lib/os/msg.h>
#include <lib/profiler/Profiler.h>
#include <lib/linalg/_kaba_export.h>
#include <Session.h>
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>
#include <data/field/MultiComponentField.h>
#include <data/grid/RegularGrid.h>
#include <data/mesh/PolygonMesh.h>
#include <data/util/ColorMap.h>
#include <graph/Graph.h>
#include <graph/NodeFactory.h>
#include <graph/draw2d/Plotter.h>
#include <graph/renderer/RendererNode.h>
#include <processing/field/Calculus.h>

#include "lib/xhui/Application.h"

extern Session* _current_session_;

Session* export_start();
void export_run();

namespace artemis {

base::map<string, Path> PluginManager::plugin_classes;


Path PluginManager::directory() {
	if (os::app::installed)
		return os::app::directory_static | "plugins";
	else
		return os::app::directory_static.parent() | "plugins";
}


void PluginManager::init() {
	kaba::config.directory = directory();
	kaba::default_context->register_package_init("artemis", directory() | "artemis", &export_kaba);
	kaba::default_context->register_package_init("linalg", directory() | "linalg", &export_package_linalg);
	import_kaba();
	find_plugins();
}

Session* current_session() {
	return _current_session_;
}

dataflow::Node* graph_add_node_by_class(graph::Graph* g, const string& _class, const vec2& pos) {
	auto n = graph::create_node(g->session, _class);
	n->pos = pos;
	g->add_node(n);
	return n;
}

bool graph_connect(dataflow::Graph* g, dataflow::Node* source, int source_port, dataflow::Node* sink, int sink_port) {
	try {
		g->connect({source, source_port, sink, sink_port});
		return true;
	} catch (Exception& e) {
		msg_error("connect failed: " + e.message());
		return false;
	}
}

void node_init(dataflow::Node* n, const string& name) {
	new(n) dataflow::Node(name);
}

template<class T>
void port_init(T* port, dataflow::Node* node, const string& name, dataflow::PortFlags flags) {
	new (port) T(node, name, flags);
}

template<class T>
void setting_init(T* setting, dataflow::Node* node, const string& name, typename base::xparam<T>::t value, const string& options) {
	new (setting) dataflow::Setting<T>(node, name, value, options);
}

template<class T>
void link_ports(kaba::Exporter* ext, const string& name) {
	ext->declare_class_size("InPort" + name, sizeof(dataflow::InPort<T>));
	ext->link_class_func("InPort" + name + ".__init__", &port_init<dataflow::InPort<T>>);
	ext->link_class_func("InPort" + name + ".value", &dataflow::InPort<T>::value);
	ext->link_class_func("InPort" + name + ".mutated", &dataflow::InPort<T>::mutated);

	ext->declare_class_size("OutPort" + name, sizeof(dataflow::OutPort<T>));
	ext->link_class_func("OutPort" + name + ".__init__", &port_init<dataflow::OutPort<T>>);
	ext->link_class_func("OutPort" + name + ".set", &dataflow::OutPort<T>::operator());
}

template<class T>
void link_setting(kaba::Exporter* ext, const string& name) {
	ext->declare_class_size("Setting" + name, sizeof(dataflow::Setting<T>));
	ext->link_class_func("Setting" + name + ".__init__", &setting_init<T>);
	ext->link_class_func("Setting" + name + "." + kaba::Identifier::func::Call, &dataflow::Setting<T>::operator());
	ext->link_class_func("Setting" + name + ".set", &dataflow::Setting<T>::set);
}

template<class T>
class GenericVDeleter : public T {
public:
	void __delete__() override {
		((T*)this)->~T();
	}
};

data::ScalarType type_kaba_to_data(const kaba::Class* type) {
	if (type == kaba::TypeFloat32)
		return data::ScalarType::Float32;
	if (type == kaba::TypeFloat64)
		return data::ScalarType::Float64;
	// in case type comes from an external kaba context...
	if (type->name == "f32")
		return data::ScalarType::Float32;
	if (type->name == "f64")
		return data::ScalarType::Float64;
	return data::ScalarType::None;
}

// FIXME will fail for external bindings :(
const kaba::Class* type_data_to_kaba(data::ScalarType type) {
	if (type == data::ScalarType::Float32)
		return kaba::TypeFloat32;
	if (type == data::ScalarType::Float64)
		return kaba::TypeFloat64;
	return kaba::TypeUnknown;
}

data::ScalarField create_scalar_field(const data::RegularGrid& grid, const kaba::Class* type, data::SamplingMode mode) {
	return data::ScalarField(grid, type_kaba_to_data(type), mode);
}

data::VectorField create_vector_field(const data::RegularGrid& grid, const kaba::Class* type, data::SamplingMode mode) {
	return data::VectorField(grid, type_kaba_to_data(type), mode);
}

data::MultiComponentField create_multi_component_field(const data::RegularGrid& grid, const kaba::Class* type, data::SamplingMode mode, int components) {
	return data::MultiComponentField(grid, type_kaba_to_data(type), mode, components);
}

template<class T>
const kaba::Class* field_get_type(const T& field) {
	return type_data_to_kaba(field.type);
}

template<class T>
void generic_assign(T& a, const T& b) {
	a = b;
}

void PluginManager::export_kaba(kaba::Exporter* ext) {

	ext->link_func("current_session", &current_session);
	ext->link_func("create_session", &create_session);
	ext->link_func("start", &export_start);
	ext->link_func("run", &export_run);
	ext->link_func("gradient", &processing::gradient);
	ext->link_func("divergence", &processing::divergence);
	ext->link_func("rotation_fw", &processing::rotation_fw);
	ext->link_func("rotation_bw", &processing::rotation_bw);
	ext->link_func("laplace", &processing::laplace);
	ext->link_func("hessian_x", &processing::hessian_x);
	ext->link_func("profiler.create_channel", &profiler::create_channel);
	ext->link_func("profiler.begin", &profiler::begin);
	ext->link_func("profiler.end", &profiler::end);

	ext->declare_class_size("Mesh", sizeof(PolygonMesh));
	ext->link_class_func("Mesh.__init__", &kaba::generic_init<PolygonMesh>);

	ext->declare_class_size("ColorMap", sizeof(data::ColorMap));
	ext->declare_class_element("ColorMap.colors", &data::ColorMap::colors);
	ext->declare_class_element("ColorMap.values", &data::ColorMap::values);

	ext->declare_class_size("ScalarField", sizeof(data::ScalarField));
	ext->declare_class_element("ScalarField.grid", &data::ScalarField::grid);
	ext->declare_class_element("ScalarField.v32", &data::ScalarField::v32);
	ext->declare_class_element("ScalarField.v64", &data::ScalarField::v64);
	ext->link_class_func("ScalarField.__init__", &kaba::generic_init<data::ScalarField>);
	ext->link_class_func("ScalarField.__delete__", &kaba::generic_delete<data::ScalarField>);
	ext->link_class_func("ScalarField.set", &data::ScalarField::set32);
	ext->link_class_func("ScalarField.value", &data::ScalarField::value32);
	ext->link_class_func("ScalarField.type", &field_get_type<data::ScalarField>);
	ext->link_class_func("ScalarField.__assign__", &generic_assign<data::ScalarField>);
	ext->link_class_func("ScalarField.__add__", &data::ScalarField::operator+);
	ext->link_class_func("ScalarField.__iadd__", &data::ScalarField::operator+=);
	ext->link_class_func("ScalarField.__sub__", &data::ScalarField::operator-);
	ext->link_class_func("ScalarField.__isub__", &data::ScalarField::operator-=);
	ext->link_class_func("ScalarField.__mul__", &data::ScalarField::operator*);
	ext->link_class_func("ScalarField.__imul__", &data::ScalarField::operator*=);
	ext->link_func("ScalarField.create", &create_scalar_field);

	ext->declare_class_size("VectorField", sizeof(data::VectorField));
	ext->link_class_func("VectorField.__init__", &kaba::generic_init<data::VectorField>);
	ext->link_class_func("VectorField.__delete__", &kaba::generic_delete<data::VectorField>);
	ext->declare_class_element("VectorField.grid", &data::VectorField::grid);
	ext->declare_class_element("VectorField.v32", &data::VectorField::v32);
	ext->declare_class_element("VectorField.v64", &data::VectorField::v64);
	ext->link_class_func("VectorField.set", &data::VectorField::set32);
	ext->link_class_func("VectorField.value", &data::VectorField::value32);
	ext->link_class_func("VectorField.type", &field_get_type<data::VectorField>);
	ext->link_class_func("VectorField.__assign__", &generic_assign<data::VectorField>);
	ext->link_class_func("VectorField.__add__", &data::VectorField::operator+);
	ext->link_class_func("VectorField.__iadd__", &data::VectorField::operator+=);
	ext->link_class_func("VectorField.__sub__", &data::VectorField::operator-);
	ext->link_class_func("VectorField.__isub__", &data::VectorField::operator-=);
	ext->link_class_func("VectorField.__mul__", &data::VectorField::operator*);
	ext->link_class_func("VectorField.__imul__", &data::VectorField::operator*=);
	ext->link_class_func("VectorField.cwise_product", &data::VectorField::componentwise_product);
	ext->link_func("VectorField.create", &create_vector_field);
	ext->link_class_func("VectorField.split", &data::VectorField::split);
	ext->link_class_func("VectorField.get_component", &data::VectorField::get_component);
	ext->link_class_func("VectorField.set_component", &data::VectorField::set_component);
	ext->link_func("VectorField.merge", &data::VectorField::merge);

	ext->declare_class_size("MultiComponentField", sizeof(data::MultiComponentField));
	ext->link_class_func("MultiComponentField.__init__", &kaba::generic_init<data::MultiComponentField>);
	ext->link_class_func("MultiComponentField.__delete__", &kaba::generic_delete<data::MultiComponentField>);
	ext->declare_class_element("MultiComponentField.grid", &data::MultiComponentField::grid);
	ext->declare_class_element("MultiComponentField.v32", &data::MultiComponentField::v32);
	ext->declare_class_element("MultiComponentField.v64", &data::MultiComponentField::v64);
	ext->link_class_func("MultiComponentField.set", &data::MultiComponentField::set32);
	ext->link_class_func("MultiComponentField.value", &data::MultiComponentField::value32);
	ext->link_class_func("MultiComponentField.type", &field_get_type<data::MultiComponentField>);
	ext->link_class_func("MultiComponentField.__assign__", &generic_assign<data::MultiComponentField>);
	ext->link_class_func("MultiComponentField.__add__", &data::MultiComponentField::operator+);
	ext->link_class_func("MultiComponentField.__iadd__", &data::MultiComponentField::operator+=);
	ext->link_class_func("MultiComponentField.__sub__", &data::MultiComponentField::operator-);
	ext->link_class_func("MultiComponentField.__isub__", &data::MultiComponentField::operator-=);
	ext->link_class_func("MultiComponentField.__mul__", &data::MultiComponentField::operator*);
	ext->link_class_func("MultiComponentField.__imul__", &data::MultiComponentField::operator*=);
	ext->link_class_func("MultiComponentField.cwise_product", &data::MultiComponentField::componentwise_product);
	ext->link_func("MultiComponentField.create", &create_multi_component_field);


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
		dataflow::Node n("");
		ext->declare_class_size("Node", sizeof(dataflow::Node));
		ext->declare_class_element("Node.name", &dataflow::Node::name);
		ext->declare_class_element("Node.channel", &dataflow::Node::channel);
		ext->declare_class_element("Node.dirty", &dataflow::Node::dirty);
		ext->declare_class_element("Node.flags", &dataflow::Node::flags);
		ext->link_class_func("Node.__init__", &node_init);
		ext->link_class_func("Node.set", &dataflow::Node::set);
		ext->link_class_func("Node.process", &dataflow::Node::process);
		ext->link_virtual("Node.__delete__", &GenericVDeleter<dataflow::Node>::__delete__, &n);
		ext->link_virtual("Node.on_process", &dataflow::Node::on_process, &n);
		ext->link_virtual("Node.create_panel", &dataflow::Node::create_panel, &n);
	}

	link_ports<artemis::data::ScalarField>(ext, "ScalarField");
	link_ports<artemis::data::VectorField>(ext, "VectorField");
	link_ports<artemis::data::MultiComponentField>(ext, "MultiComponentField");
	link_ports<artemis::data::RegularGrid>(ext, "RegularGrid");
	link_ports<Array<double>>(ext, "List");

	link_setting<double>(ext, "Float");
	link_setting<int>(ext, "Int");
	link_setting<bool>(ext, "Bool");
	link_setting<string>(ext, "String");
	link_setting<color>(ext, "Color");

	ext->declare_class_size("Graph", sizeof(dataflow::Graph));
	ext->declare_class_element("Graph.t", &artemis::graph::Graph::t);
	ext->declare_class_element("Graph.dt", &artemis::graph::Graph::dt);
	ext->link_class_func("Graph.add_node", &graph_add_node_by_class);
	ext->link_class_func("Graph.connect", &graph_connect);

}

template<class C>
void import_component_class(shared<kaba::Module> m, const string &name) {
	for (auto c: m->classes())
		if (c->name == name) {
			dataflow::type_map.set(&typeid(C), c);
			return;
		}
	throw Exception(format("artemis.kaba: %s missing", name));
}

void PluginManager::import_kaba() {
	dataflow::link_basic_types();

	auto m = kaba::default_context->load_module("artemis/artemis.kaba");
	import_component_class<PolygonMesh>(m, "Mesh");
	import_component_class<data::RegularGrid>(m, "RegularGrid");
	import_component_class<data::ScalarField>(m, "ScalarField");
	import_component_class<data::VectorField>(m, "VectorField");
	import_component_class<data::MultiComponentField>(m, "MultiComponentField");
	import_component_class<data::ColorMap>(m, "ColorMap");
	import_component_class<graph::PlotData>(m, "PlotData");
	import_component_class<graph::RenderData>(m, "RenderData");
	import_component_class<data::SamplingMode>(m, "SamplingMode");
}

void PluginManager::find_plugins() {
	Path dir = directory() | "nodes";
	for (const auto& e: os::fs::search(dir, "*.kaba", "fr")) {
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

