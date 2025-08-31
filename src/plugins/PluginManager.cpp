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
#include <lib/fft/_kaba_export.h>
#include <Session.h>
#include <data/field/ScalarField.h>
#include <data/field/VectorField.h>
#include <data/field/MultiComponentField.h>
#include <data/grid/Grid.h>
#include <data/grid/RegularGrid.h>
#include <lib/mesh/PolygonMesh.h>
#include <data/util/ColorMap.h>
#include <graph/Graph.h>
#include <graph/NodeFactory.h>
#include <graph/draw2d/Plotter.h>
#include <graph/renderer/RendererNode.h>
#include <processing/field/Calculus.h>

extern Session* _current_session_;

Session* export_start();
void export_run();

namespace artemis {

base::map<string, Path> PluginManager::plugin_classes;
Array<string> PluginManager::template_classes;


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
	kaba::default_context->register_package_init("fft", directory() | "fft", &export_package_fft);
	import_kaba();
	find_plugins();
	find_templates();
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
	auto r = g->connect({source, source_port, sink, sink_port});
	if (!r)
		msg_error("connect failed: " + r.error().msg);
	return r.has_value();
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
	ext->declare_class_element("ScalarField.sampling_mode", &data::ScalarField::sampling_mode);
	ext->declare_class_element("ScalarField.v32", &data::ScalarField::v32);
	ext->declare_class_element("ScalarField.v64", &data::ScalarField::v64);
	ext->link_class_func("ScalarField.__init__", &kaba::generic_init<data::ScalarField>);
	ext->link_class_func("ScalarField.__delete__", &kaba::generic_delete<data::ScalarField>);
	ext->link_class_func("ScalarField.value", &data::ScalarField::value);
	ext->link_class_func("ScalarField.set", &data::ScalarField::set);
	ext->link_class_func("ScalarField._set", &data::ScalarField::_set);
	ext->link_class_func("ScalarField._value", &data::ScalarField::_value);
	ext->link_class_func("ScalarField.average", &data::ScalarField::average);
	ext->link_class_func("ScalarField.min", &data::ScalarField::min);
	ext->link_class_func("ScalarField.max", &data::ScalarField::max);
	ext->link_class_func("ScalarField.type", &field_get_type<data::ScalarField>);
	ext->link_class_func("ScalarField.__assign__:ScalarField:ScalarField", &kaba::generic_assign<data::ScalarField>);
	ext->link_class_func("ScalarField.__assign__:ScalarField:f64", &kaba::generic_assign<data::ScalarField, double>);
	ext->link_class_func("ScalarField.__add__:ScalarField:ScalarField", &kaba::generic_add<data::ScalarField>);
	ext->link_class_func("ScalarField.__add__:ScalarField:f64", &kaba::generic_add<data::ScalarField, double>);
	ext->link_class_func("ScalarField.__iadd__:ScalarField:ScalarField", &kaba::generic_iadd<data::ScalarField>);
	ext->link_class_func("ScalarField.__iadd__:ScalarField:f64", &kaba::generic_iadd<data::ScalarField, double>);
	ext->link_class_func("ScalarField.__sub__:ScalarField:ScalarField", &kaba::generic_sub<data::ScalarField>);
	ext->link_class_func("ScalarField.__sub__:ScalarField:f64", &kaba::generic_sub<data::ScalarField, double>);
	ext->link_class_func("ScalarField.__isub__:ScalarField:ScalarField", &kaba::generic_isub<data::ScalarField>);
	ext->link_class_func("ScalarField.__isub__:ScalarField:f64", &kaba::generic_isub<data::ScalarField, double>);
	ext->link_class_func("ScalarField.__mul__:ScalarField:ScalarField", &kaba::generic_mul<data::ScalarField>);
	ext->link_class_func("ScalarField.__mul__:ScalarField:f64", &kaba::generic_mul<data::ScalarField, double>);
	ext->link_class_func("ScalarField.__imul__:ScalarField:ScalarField", &kaba::generic_imul<data::ScalarField>);
	ext->link_class_func("ScalarField.__imul__:ScalarField:f64", &kaba::generic_imul<data::ScalarField, double>);
	ext->link_func("ScalarField.create", &create_scalar_field);

	ext->declare_class_size("VectorField", sizeof(data::VectorField));
	ext->link_class_func("VectorField.__init__", &kaba::generic_init<data::VectorField>);
	ext->link_class_func("VectorField.__delete__", &kaba::generic_delete<data::VectorField>);
	ext->declare_class_element("VectorField.grid", &data::VectorField::grid);
	ext->declare_class_element("VectorField.sampling_mode", &data::VectorField::sampling_mode);
	ext->declare_class_element("VectorField.v32", &data::VectorField::v32);
	ext->declare_class_element("VectorField.v64", &data::VectorField::v64);
	ext->link_class_func("VectorField.set", &data::VectorField::set32);
	ext->link_class_func("VectorField.value", &data::VectorField::value32);
	ext->link_class_func("VectorField._set", &data::VectorField::_set32);
	ext->link_class_func("VectorField._value", &data::VectorField::_value32);
	ext->link_class_func("VectorField.average", &data::VectorField::average32);
	ext->link_class_func("VectorField.type", &field_get_type<data::VectorField>);
	ext->link_class_func("VectorField.__assign__:VectorField:VectorField", &kaba::generic_assign<data::VectorField>);
	ext->link_class_func("VectorField.__assign__:VectorField:math.vec3", &kaba::generic_assign<data::VectorField, const vec3&>);
	ext->link_class_func("VectorField.__add__", &data::VectorField::operator+);
	ext->link_class_func("VectorField.__iadd__:VectorField:VectorField", &data::VectorField::operator+=);
	ext->link_class_func("VectorField.__iadd__:VectorField:math.vec3", &data::VectorField::iadd_single32);
	ext->link_class_func("VectorField.__sub__", &data::VectorField::operator-);
	ext->link_class_func("VectorField.__isub__:VectorField:VectorField", &data::VectorField::operator-=);
	ext->link_class_func("VectorField.__isub__:VectorField:math.vec3", &data::VectorField::isub_single32);
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
	ext->declare_class_element("MultiComponentField.sampling_mode", &data::MultiComponentField::sampling_mode);
	ext->declare_class_element("MultiComponentField.components", &data::MultiComponentField::components);
	ext->declare_class_element("MultiComponentField.v32", &data::MultiComponentField::v32);
	ext->declare_class_element("MultiComponentField.v64", &data::MultiComponentField::v64);
	ext->link_class_func("MultiComponentField.set", &data::MultiComponentField::set);
	ext->link_class_func("MultiComponentField.value", &data::MultiComponentField::value);
	ext->link_class_func("MultiComponentField.values", &data::MultiComponentField::values);
	ext->link_class_func("MultiComponentField._set", &data::MultiComponentField::_set);
	ext->link_class_func("MultiComponentField._value", &data::MultiComponentField::_value);
	ext->link_class_func("MultiComponentField.type", &field_get_type<data::MultiComponentField>);
	ext->link_class_func("MultiComponentField.__assign__", &kaba::generic_assign<data::MultiComponentField>);
	ext->link_class_func("MultiComponentField.__add__", &data::MultiComponentField::operator+);
	ext->link_class_func("MultiComponentField.__iadd__", &data::MultiComponentField::operator+=);
	ext->link_class_func("MultiComponentField.__sub__", &data::MultiComponentField::operator-);
	ext->link_class_func("MultiComponentField.__isub__", &data::MultiComponentField::operator-=);
	ext->link_class_func("MultiComponentField.__mul__", &data::MultiComponentField::operator*);
	ext->link_class_func("MultiComponentField.__imul__", &data::MultiComponentField::operator*=);
	ext->link_class_func("MultiComponentField.cwise_product", &data::MultiComponentField::componentwise_product);
	ext->link_func("MultiComponentField.create", &create_multi_component_field);


	ext->declare_class_size("RegularGrid", sizeof(data::RegularGrid));
	ext->declare_class_element("RegularGrid.nx", &data::RegularGrid::nx);
	ext->declare_class_element("RegularGrid.ny", &data::RegularGrid::ny);
	ext->declare_class_element("RegularGrid.nz", &data::RegularGrid::nz);
	ext->declare_class_element("RegularGrid.dx", &data::RegularGrid::dx);
	ext->declare_class_element("RegularGrid.dy", &data::RegularGrid::dy);
	ext->declare_class_element("RegularGrid.dz", &data::RegularGrid::dz);
	ext->link_class_func("RegularGrid.__init__", &kaba::generic_init<data::RegularGrid>);
	ext->link_class_func("RegularGrid.points", &data::RegularGrid::points);
	ext->link_class_func("RegularGrid.count", &data::RegularGrid::count);

	ext->declare_class_size("Grid", sizeof(data::Grid));
	ext->declare_class_element("Grid.regular", &data::Grid::regular);
	ext->link_class_func("Grid.__init__", &kaba::generic_init<data::Grid>);
	ext->link_class_func("Grid.points", &data::Grid::points);
	ext->link_class_func("Grid.count", &data::Grid::count);

	ext->declare_class_size("Session", sizeof(Session));
	ext->declare_class_element("Session.graph", &Session::graph);


	{
		dataflow::Node n("");
		ext->declare_class_size("Node", sizeof(dataflow::Node));
		ext->declare_class_element("Node.name", &dataflow::Node::name);
		ext->declare_class_element("Node.channel", &dataflow::Node::channel);
		ext->declare_class_element("Node._state", &dataflow::Node::state);
		ext->declare_class_element("Node.flags", &dataflow::Node::flags);
		ext->link_class_func("Node.__init__", &node_init);
		ext->link_class_func("Node.set", &dataflow::Node::set);
		ext->link_class_func("Node.process", &dataflow::Node::process);
		ext->link_virtual("Node.__delete__", &GenericVDeleter<dataflow::Node>::__delete__, &n);
		ext->link_virtual("Node.additional_init", &dataflow::Node::additional_init, &n);
		ext->link_virtual("Node.on_process", &dataflow::Node::on_process, &n);
		ext->link_virtual("Node.create_panel", &dataflow::Node::create_panel, &n);
	}
	{
		ext->declare_class_size("GenericData", sizeof(dataflow::GenericData));
		ext->declare_class_element("GenericData.type", &dataflow::GenericData::type);
		ext->declare_class_element("GenericData.p", &dataflow::GenericData::p);
	}
	{
		ext->declare_class_size("InPortBase", sizeof(dataflow::InPortBase));
		ext->link_class_func("InPortBase.__init__", &kaba::generic_init_ext<dataflow::InPortBase, dataflow::Node*, const string&, const kaba::Class*, dataflow::PortFlags>);
		ext->link_class_func("InPortBase.mutated", &dataflow::InPortBase::mutated);
		ext->link_class_func("InPortBase.generic_values", &dataflow::InPortBase::generic_values);
	}
	{
		ext->declare_class_size("OutPortBase", sizeof(dataflow::OutPortBase));
		ext->link_class_func("OutPortBase.__init__", &kaba::generic_init_ext<dataflow::OutPortBase, dataflow::Node*, const string&, const kaba::Class*, void*, dataflow::PortFlags>);
		ext->link_class_func("OutPortBase.generic_set", &dataflow::OutPortBase::generic_set);
	}

	link_ports<data::ScalarField>(ext, "ScalarField");
	link_ports<data::VectorField>(ext, "VectorField");
	link_ports<data::MultiComponentField>(ext, "MultiComponentField");
	link_ports<data::Grid>(ext, "Grid");
	link_ports<double>(ext, "Float");
	link_ports<vec3>(ext, "Vector");
	link_ports<Array<double>>(ext, "List");
	link_ports<Array<vec3>>(ext, "VectorList");

	link_setting<double>(ext, "Float");
	link_setting<int>(ext, "Int");
	link_setting<bool>(ext, "Bool");
	link_setting<string>(ext, "String");
	link_setting<color>(ext, "Color");

	ext->declare_class_size("Graph", sizeof(dataflow::Graph));
	ext->declare_class_element("Graph.t", &graph::Graph::t);
	ext->declare_class_element("Graph.dt", &graph::Graph::dt);
	ext->link_class_func("Graph.add_node", &graph_add_node_by_class);
	ext->link_class_func("Graph.connect", &graph_connect);
	ext->link_class_func("Graph.clear", &graph::Graph::clear);

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
	auto mdata = kaba::default_context->load_module("artemis/data.kaba");
	auto mgrid = kaba::default_context->load_module("artemis/grid.kaba");
	auto mfields = kaba::default_context->load_module("artemis/fields.kaba");
	auto mgraph = kaba::default_context->load_module("artemis/graph.kaba");
	import_component_class<PolygonMesh>(mdata, "Mesh");
	import_component_class<data::Grid>(mgrid, "Grid");
	import_component_class<data::RegularGrid>(mgrid, "RegularGrid");
	import_component_class<data::ScalarField>(mfields, "ScalarField");
	import_component_class<data::VectorField>(mfields, "VectorField");
	import_component_class<data::MultiComponentField>(mfields, "MultiComponentField");
	import_component_class<data::ColorMap>(mdata, "ColorMap");
	import_component_class<graph::PlotData>(mdata, "PlotData");
	import_component_class<graph::RenderData>(mdata, "RenderData");
	import_component_class<graph::DrawCall>(mdata, "DrawCall");
	import_component_class<data::SamplingMode>(mgrid, "SamplingMode");
}

void PluginManager::find_plugins() {
	Path dir = directory() | "nodes";
	for (const auto& e: os::fs::search(dir, "*.kaba", "fr")) {
		string name = e.basename_no_ext();
		plugin_classes.set(name, dir | e);
	}
}

void* PluginManager::create_instance(const string& name) {
	for (const auto& [n, f] : plugin_classes)
		if (name == n) {
			auto m = kaba::default_context->load_module(f);
			for (const auto c: m->classes())
				if (c->name == name)
					return c->create_instance();
		}
	msg_error("NOT FOUND: " + name);
	return nullptr;
}

Path PluginManager::template_directory() {
	return os::app::directory_dynamic | "templates";
}


void PluginManager::find_templates() {
	os::fs::create_directory(template_directory());
	for (const auto& e: os::fs::search(template_directory(), "*.artemis", "f")) {
		string name = e.basename_no_ext();
		template_classes.add(name);
	}
}


}

