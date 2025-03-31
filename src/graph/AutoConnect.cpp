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

void connect_through(Graph* g, const dataflow::CableInfo& c, const Array<string>& glue_nodes) {
	dataflow::Node* source = c.source;
	int source_port = c.source_port;
	for (const auto& [i, glue]: enumerate(glue_nodes)) {
		auto n = create_node(g->session, glue);
		n->pos = c.source->pos + (c.sink->pos - c.source->pos) / (float)(glue_nodes.num + 1) * (float)(i + 1);
		g->add_node(n);
		g->connect({source, source_port, n, 0});
		source = n;
		source_port = 0;
	}
	g->connect({source, source_port, c.sink, c.sink_port});
}

void auto_connect(Graph* g, const dataflow::CableInfo& c) {
	auto source = c.source->out_ports[c.source_port];
	auto sink = c.sink->in_ports[c.sink_port];
	if (source->class_ == sink->class_) {
		g->connect(c);
	} else if (source->class_->name == "Mesh" and sink->class_->name == "RenderData") {
		connect_through(g, c, {"MeshRenderer"});
	} else if (source->class_->name == "RegularGrid" and sink->class_->name == "RenderData") {
		connect_through(g, c, {"GridRenderer"});
	} else if (source->class_->name == "ScalarField" and sink->class_->name == "RenderData") {
		connect_through(g, c, {"VolumeRenderer"});
	} else if (source->class_->name == "VectorField" and sink->class_->name == "RenderData") {
		connect_through(g, c, {"VectorFieldRenderer"});
	} else if (source->class_->name == "vec3[]" and sink->class_->name == "RenderData") {
		connect_through(g, c, {"PointsRenderer"});
	} else if (source->class_->name == "PlotData" and sink->class_->name == "RenderData") {
		connect_through(g, c, {"Plotter"});
	} else if (source->class_->name == "f32[]" and sink->class_->name == "RenderData") {
		connect_through(g, c, {"ListPlot", "Plotter"});
	} else if (source->class_->name == "f32[]" and sink->class_->name == "PlotData") {
		connect_through(g, c, {"ListPlot"});
	} else {
		throw Exception(format("can not connect  %s  to  %s", source->class_->name, sink->class_->name));
	}
}

} // graph