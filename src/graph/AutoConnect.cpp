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
#include <lib/kaba/syntax/Class.h>

namespace artemis {
	Session* current_session();
}

namespace artemis::graph {

base::expected<int> connect_through(dataflow::Graph* g, const dataflow::CableInfo& c, const Array<string>& glue_nodes) {
	dataflow::Node* source = c.source->owner;
	vec2 pos0 = c.source->owner->pos;
	vec2 pos1 = c.sink->owner->pos;
	int source_port = c.source_port_no();
	for (const auto& [i, glue]: enumerate(glue_nodes)) {
		auto n = create_node(current_session(), glue);
		n->pos = pos0 + (pos1 - pos0) / (float)(glue_nodes.num + 1) * (float)(i + 1);
		g->add_node(n);
		auto r = g->connect({source->out_ports[source_port], n->in_ports[0]});
		if (!r)
			return r;
		source = n;
		source_port = 0;
	}
	return g->connect({source->out_ports[source_port], c.sink});
}

base::expected<int> auto_connect(dataflow::Graph* g, const dataflow::CableInfo& c) {
	auto source = c.source;
	auto sink = c.sink;
	if (dataflow::port_type_match(*source, *sink))
		return g->connect(c);
	// TODO recursive "solver"
	if (source->type->name == "Mesh" and sink->type->name == "DrawCall")
		return connect_through(g, c, {"MeshRenderer"});
	if (source->type->name == "Mesh" and sink->type->name == "RenderData")
		return connect_through(g, c, {"MeshRenderer", "SceneRenderer"});
	if (source->type->name == "DrawCall" and sink->type->name == "RenderData")
		return connect_through(g, c, {"SceneRenderer"});
	if (source->type->name == "Grid" and sink->type->name == "DrawCall")
		return connect_through(g, c, {"GridRenderer"});
	if (source->type->name == "Grid" and sink->type->name == "RenderData")
		return connect_through(g, c, {"GridRenderer", "SceneRenderer"});
	if (source->type->name == "ScalarField" and sink->type->name == "DrawCall")
		return connect_through(g, c, {"VolumeRenderer"});
	if (source->type->name == "ScalarField" and sink->type->name == "RenderData")
		return connect_through(g, c, {"VolumeRenderer", "SceneRenderer"});
	if (source->type->name == "VectorField" and sink->type->name == "DrawCall")
		return connect_through(g, c, {"VectorFieldRenderer"});
	if (source->type->name == "VectorField" and sink->type->name == "RenderData")
		return connect_through(g, c, {"VectorFieldRenderer", "SceneRenderer"});
	if (source->type->name == "vec3[]" and sink->type->name == "DrawCall")
		return connect_through(g, c, {"PointListRenderer"});
	if (source->type->name == "vec3[]" and sink->type->name == "RenderData")
		return connect_through(g, c, {"PointListRenderer", "SceneRenderer"});
	if (source->type->name == "PlotData" and sink->type->name == "RenderData")
		return connect_through(g, c, {"Plotter"});
	if (source->type->name == "f64[]" and sink->type->name == "RenderData")
		return connect_through(g, c, {"ListPlot", "Plotter"});
	if (source->type->name == "f64[]" and sink->type->name == "PlotData")
		return connect_through(g, c, {"ListPlot"});
	return base::Error{format("can not connect  <b>%s</b>  to  <b>%s</b>", source->type->name, sink->type->name)};
}

} // graph