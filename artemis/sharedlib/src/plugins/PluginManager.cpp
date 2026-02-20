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

#include "lib/mesh/GeometrySphere.h"
#include "lib/mesh/GeometryTeapot.h"

extern Session* _current_session_;

void start_session_load_file(Session* s, const Path& filename);
void start_session_empty(Session* s);
void app_run();

namespace artemis {

base::map<string, Path> PluginManager::plugin_classes;
Array<string> PluginManager::template_classes;
kaba::Package* PluginManager::package;


Path PluginManager::directory() {
	return package->directory;
	/*if (os::app::installed)
		return os::app::directory_static | "plugins";
	else
		return os::app::directory_static.parent() | "plugins";*/
}


void PluginManager::init() {
	package = kaba::default_context->get_package("artemis");
	//kaba::config.directory = directory();

	import_kaba();
	find_plugins();
	find_templates();
}

Session* current_session() {
	return _current_session_;
}

dataflow::Node* graph_add_node_by_class(dataflow::Graph* g, const string& _class, const vec2& pos) {
	auto n = graph::create_node(current_session(), _class);
	n->pos = pos;
	g->add_node(n);
	return n;
}

dataflow::Graph* graph_group_nodes(dataflow::Graph* g, const Array<dataflow::Node*>& nodes) {
	base::set<dataflow::Node*> _nodes;
	for (auto n: nodes)
		_nodes.add(n);
	return g->group_nodes(_nodes);
}

void test_done() {
	msg_write("DONE");
	exit(0);
}

bool graph_connect(dataflow::Graph* g, dataflow::Node* source, int source_port, dataflow::Node* sink, int sink_port) {
	auto r = g->connect(source, source_port, sink, sink_port);
	if (!r)
		msg_error("connect failed: " + r.error().msg);
	return r.has_value();
}

void node_init(dataflow::Node* n, const string& name) {
	new(n) dataflow::Node(name);
}

template<class T>
class GenericVDeleter : public T {
public:
	void __delete__() override {
		((T*)this)->~T();
	}
};

data::ScalarType type_kaba_to_data(const kaba::Class* type) {
	if (type == kaba::common_types.f32)
		return data::ScalarType::Float32;
	if (type == kaba::common_types.f64)
		return data::ScalarType::Float64;
	return data::ScalarType::None;
}

// FIXME will fail for external bindings :(
const kaba::Class* type_data_to_kaba(data::ScalarType type) {
	if (type == data::ScalarType::Float32)
		return kaba::common_types.f32;
	if (type == data::ScalarType::Float64)
		return kaba::common_types.f64;
	return kaba::common_types.unknown;
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
	ext->package_info("artemis", "0.5");

	ext->link_func("current_session", &current_session);
	ext->link_func("create_session", &create_session);
	ext->link_func("start_session_load_file", &start_session_load_file);
	ext->link_func("start_session_empty", &start_session_empty);
	ext->link_func("app_run", &app_run);
	ext->link_func("test_done", &test_done);
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
	ext->link_class_func("Mesh.__delete__", &kaba::generic_delete<PolygonMesh>);
	ext->link_class_func("Mesh.__assign__", &kaba::generic_assign<PolygonMesh>);
	ext->link_class_func("Mesh.smoothen", &PolygonMesh::smoothen);

	ext->link_func("Mesh.create_teapot", &GeometryTeapot::create);
	ext->link_func("Mesh.create_sphere", &GeometrySphere::create);

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
	ext->declare_class_element("MultiComponentField.buffer", &data::MultiComponentField::buffer);
	ext->link_class_func("MultiComponentField.set", &data::MultiComponentField::set);
	ext->link_class_func("MultiComponentField.value", &data::MultiComponentField::value);
	ext->link_class_func("MultiComponentField.values", &data::MultiComponentField::values);
	ext->link_class_func("MultiComponentField._set", &data::MultiComponentField::_set);
	ext->link_class_func("MultiComponentField._value", &data::MultiComponentField::_value);
	ext->link_class_func("MultiComponentField.type", &field_get_type<data::MultiComponentField>);
	ext->link_class_func("MultiComponentField.begin_edit_cpu", &data::MultiComponentField::begin_edit_cpu);
	ext->link_class_func("MultiComponentField.begin_edit_gpu", &data::MultiComponentField::begin_edit_gpu);
	ext->link_class_func("MultiComponentField.begin_read_cpu", &data::MultiComponentField::begin_read_cpu);
	ext->link_class_func("MultiComponentField.begin_read_gpu", &data::MultiComponentField::begin_read_gpu);
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
	ext->declare_class_element("Session.t", &Session::t);
	ext->declare_class_element("Session.dt", &Session::dt);


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
		ext->link_class_func("InPortBase.__init2__", &kaba::generic_init_ext<dataflow::InPortBase, dataflow::Node*, const string&, const kaba::Class*, dataflow::PortFlags>);
		ext->link_class_func("InPortBase.mutated", &dataflow::InPortBase::mutated);
		ext->link_class_func("InPortBase.generic_values", &dataflow::InPortBase::generic_values);
	}
	{
		ext->declare_class_size("OutPortBase", sizeof(dataflow::OutPortBase));
		ext->declare_class_element("OutPortBase.has_value", &dataflow::OutPortBase::has_value);
		ext->link_class_func("OutPortBase.__init2__", &kaba::generic_init_ext<dataflow::OutPortBase, dataflow::Node*, const string&, const kaba::Class*, void*, dataflow::PortFlags>);
		ext->link_class_func("OutPortBase.generic_set", &dataflow::OutPortBase::generic_set);
		ext->link_class_func("OutPortBase.mutated", &dataflow::OutPortBase::mutated);
	}

	ext->declare_class_size("SettingBase", sizeof(dataflow::SettingBase));
	ext->declare_class_element("SettingBase.name", &dataflow::SettingBase::name);
	ext->declare_class_element("SettingBase.type", &dataflow::SettingBase::type);
	ext->declare_class_element("SettingBase.generic_value_pointer", &dataflow::SettingBase::generic_value_pointer);
	ext->link_class_func("SettingBase.__init2__", &kaba::generic_init_ext<dataflow::SettingBase, dataflow::Node*, const string&, const kaba::Class*, void*, const string&>);
	ext->link_class_func("SettingBase.__delete__", &kaba::generic_delete<dataflow::SettingBase>);
	ext->link_class_func("SettingBase.generic_set", &dataflow::SettingBase::generic_set);

	ext->declare_class_size("Graph", sizeof(dataflow::Graph));
	ext->link_class_func("Graph.add_node", &graph_add_node_by_class);
	ext->link_class_func("Graph.connect", &graph_connect);
	ext->link_class_func("Graph.clear", &dataflow::Graph::clear);
	ext->link_class_func("Graph.iterate", &dataflow::Graph::iterate);
	ext->link_class_func("Graph.group_nodes", &graph_group_nodes);

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

	const auto dir = package->directory;

	auto m = kaba::default_context->dll_load_module(dir | "artemis.kaba");
	auto mdata = kaba::default_context->dll_load_module(dir | "data.kaba");
	auto mgrid = kaba::default_context->dll_load_module(dir | "grid.kaba");
	auto mfields = kaba::default_context->dll_load_module(dir | "fields.kaba");
	auto mgraph = kaba::default_context->dll_load_module(dir | "graph.kaba");
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
			auto m = kaba::default_context->dll_load_module(f);
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

