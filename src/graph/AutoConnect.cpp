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

namespace artemis::graph {

base::expected<int> connect_through(Graph* g, const dataflow::CableInfo& c, const Array<string>& glue_nodes) {
	dataflow::Node* source = c.source;
	int source_port = c.source_port;
	for (const auto& [i, glue]: enumerate(glue_nodes)) {
		auto n = create_node(g->session, glue);
		n->pos = c.source->pos + (c.sink->pos - c.source->pos) / (float)(glue_nodes.num + 1) * (float)(i + 1);
		g->add_node(n);
		auto r = g->connect({source, source_port, n, 0});
		if (!r)
			return r;
		source = n;
		source_port = 0;
	}
	return g->connect({source, source_port, c.sink, c.sink_port});
}

base::expected<int> auto_connect(Graph* g, const dataflow::CableInfo& c) {
	auto source = c.source->out_ports[c.source_port];
	auto sink = c.sink->in_ports[c.sink_port];
	if (dataflow::port_type_match(*source, *sink))
		return g->connect(c);
	if (source->class_->name == "Mesh" and sink->class_->name == "RenderData")
		return connect_through(g, c, {"MeshRenderer"});
	if (source->class_->name == "Grid" and sink->class_->name == "RenderData")
		return connect_through(g, c, {"GridRenderer"});
	if (source->class_->name == "ScalarField" and sink->class_->name == "RenderData")
		return connect_through(g, c, {"VolumeRenderer"});
	if (source->class_->name == "VectorField" and sink->class_->name == "RenderData")
		return connect_through(g, c, {"VectorFieldRenderer"});
	if (source->class_->name == "vec3[]" and sink->class_->name == "RenderData")
		return connect_through(g, c, {"PointListRenderer"});
	if (source->class_->name == "PlotData" and sink->class_->name == "RenderData")
		return connect_through(g, c, {"Plotter"});
	if (source->class_->name == "f64[]" and sink->class_->name == "RenderData")
		return connect_through(g, c, {"ListPlot", "Plotter"});
	if (source->class_->name == "f64[]" and sink->class_->name == "PlotData")
		return connect_through(g, c, {"ListPlot"});
	return base::Error{format("can not connect  <b>%s</b>  to  <b>%s</b>", source->class_->name, sink->class_->name)};
}

} // graph