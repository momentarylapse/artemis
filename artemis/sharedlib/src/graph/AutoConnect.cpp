//
// Created by Michael Ankele on 2025-03-17.
//

#include "AutoConnect.h"
#include "Graph.h"
#include "NodeFactory.h"
#include <lib/base/iter.h>
#include <lib/dataflow/Node.h>
#include <lib/dataflow/Graph.h>
#include <lib/dataflow/Port.h>
#include <lib/kapi/kapi.h>

namespace artemis {
	Session* current_session();
}

namespace artemis::graph {

base::expected<Array<string>> find_auto_connect_glue_nodes(dataflow::Graph* g, const dataflow::CableInfo& c) {
	using R = Array<string>;
	auto source = c.source;
	auto sink = c.sink;
	if (dataflow::port_type_match(*source, *sink))
		return R{};
	string source_name = kaba::default_context->type_name(source->type);
	string sink_name = kaba::default_context->type_name(sink->type);
	// TODO recursive "solver"
	if (source_name == "Mesh" and sink_name == "DrawCall3d")
		return R{"MeshRenderer"};
	if (source_name == "Mesh" and sink_name == "RenderData")
		return R{"MeshRenderer", "SceneRenderer"};
	if (source_name == "Image" and sink_name == "DrawCall3d")
		return R{"ImageRenderer"};
	if (source_name == "Image" and sink_name == "RenderData")
		return R{"ImageRenderer", "SceneRenderer"};
	if (source_name == "DrawCall3d" and sink_name == "RenderData")
		return R{"SceneRenderer"};
	if (source_name == "Grid" and sink_name == "DrawCall3d")
		return R{"GridRenderer"};
	if (source_name == "Grid" and sink_name == "RenderData")
		return R{"GridRenderer", "SceneRenderer"};
	if (source_name == "ScalarField" and sink_name == "DrawCall3d")
		return R{"VolumeRenderer"};
	if (source_name == "ScalarField" and sink_name == "RenderData")
		return R{"VolumeRenderer", "SceneRenderer"};
	if (source_name == "VectorField" and sink_name == "DrawCall3d")
		return R{"VectorFieldRenderer"};
	if (source_name == "VectorField" and sink_name == "RenderData")
		return R{"VectorFieldRenderer", "SceneRenderer"};
	if (source_name == "vec3[]" and sink_name == "DrawCall3d")
		return R{"PointListRenderer"};
	if (source_name == "vec3[]" and sink_name == "RenderData")
		return R{"PointListRenderer", "SceneRenderer"};
	if (source_name == "PlotData" and sink_name == "RenderData")
		return R{"Plotter"};
	if (source_name == "f64[]" and sink_name == "RenderData")
		return R{"ListPlot", "Plotter"};
	if (source_name == "f64[]" and sink_name == "PlotData")
		return R{"ListPlot"};
	return base::Error{format("can not connect  <b>%s</b>  to  <b>%s</b>", source_name, sink_name)};
}

base::expected<GraphUpdate> find_auto_connect(dataflow::Graph* g, const dataflow::CableInfo& c) {
	auto glue_nodes = find_auto_connect_glue_nodes(g, c);
	if (!glue_nodes)
		return glue_nodes.error();

	GraphUpdate update;

	dataflow::Node* source = c.source->owner;
	vec2 pos0 = c.source->owner->pos;
	vec2 pos1 = c.sink->owner->pos;
	int source_port = c.source_port_no();
	for (const auto& [i, glue]: enumerate(*glue_nodes)) {
		auto n = create_node(current_session(), glue);
		n->pos = pos0 + (pos1 - pos0) / (float)(glue_nodes->num + 1) * (float)(i + 1);
		update.nodes.add(n);
		update.cables.add({source->out_ports[source_port], n->in_ports[0]});
		source = n;
		source_port = 0;
	}
	update.cables.add({source->out_ports[source_port], c.sink});
	return update;
}

base::expected<int> auto_connect(dataflow::Graph* g, const dataflow::CableInfo& c) {
	// solve
	auto update = find_auto_connect(g, c);
	if (!update)
		return update.error();

	// apply
	for (auto n: update->nodes)
		g->add_node(n);
	for (const auto& c: update->cables)
		if (auto r = g->connect(c); !r)
			return r;
	return 0;
}

} // graph