/*
 * FormatArtemis.cpp
 *
 *  Created on: 03.06.2025
 *      Author: michi
 */

#include "FormatArtemis.h"
#include "../../Session.h"
#include <lib/base/iter.h>
#include "../../lib/math/vec2.h"
#include "../../lib/math/vec3.h"
#include "../../lib/os/msg.h"
#include "../../lib/doc/xml.h"
#include "graph/NodeFactory.h"
#include "lib/any/any.h"
#include "lib/dataflow/Node.h"
#include "lib/dataflow/Port.h"
#include "lib/dataflow/Setting.h"
#include "lib/kaba/syntax/Class.h"
#include "world/World.h"

static string _(const string &s) { return s; }

FormatArtemis::FormatArtemis(Session *s) : TypedFormat<artemis::graph::DataGraph>(s, -1, "artemis", _("Artemis workspace"), Flag::CANONICAL_READ_WRITE) {
}



static string v2s(const vec2 &v) {
	return format("%.3f %.3f", v.x, v.y);
}

static vec2 s2v(const string &s) {
	auto x = s.explode(" ");
	if (x.num >= 2)
		return vec2(x[0]._float(), x[1]._float());
	return {0,0};
}

void FormatArtemis::_load(const Path &filename, artemis::graph::DataGraph* data, bool deep) {
	data->reset();

	xml::Parser p;
	p.load(filename);
	if (auto* g = p.elements[0].find("graph")) {
		for (auto &e: g->elements) {
			if (e.tag == "node") {
				auto n = artemis::graph::create_node(data->session, e.value("class"));
				n->pos = s2v(e.value("pos"));
				data->graph->add_node(n);
				for (const auto& ee: e.elements)
					n->set(ee.tag, Any::parse(ee.text));
			} else if (e.tag == "cable") {
				int id0 = e.value("source")._int();
				int id1 = e.value("sink")._int();
				if (id0 >= 0 and id1 >= 0 and id0 < data->graph->nodes.num and id1 < data->graph->nodes.num) {
					auto source = data->graph->nodes[id0];
					auto sink = data->graph->nodes[id1];
					data->graph->connect({source, e.value("source_port")._int(), sink, e.value("sink_port")._int()});
				}
			}
		}
	}
}


void FormatArtemis::_save(const Path &filename, artemis::graph::DataGraph *data) {

	xml::Parser p;
	p.elements.add(xml::Element("workspace"));
	auto &w = p.elements[0];

	{
		auto h = xml::Element("header");
		h.add(xml::Element("version", "1.0"));
		w.add(h);
	}

	{
		auto g = xml::Element("graph");

		for (const auto& n: data->graph->nodes) {
			auto nn = xml::Element("node").witha("class", n->name).witha("pos", v2s(n->pos));
			for (const auto s: n->settings)
				nn.add(xml::Element(s->name, s->get_generic().repr()));
			g.add(nn);
		}

		for (const auto& c: data->graph->cables()) {
			int id0 = data->graph->nodes.find(c.source);
			int id1 = data->graph->nodes.find(c.sink);
			if (id0 < 0 or id1 < 0) // we might be a group with exterior connections...
				continue;
			auto cc = xml::Element("cable").witha("source", str(id0)).witha("source_port", str(c.source_port)).witha("sink", str(id1)).witha("sink_port", str(c.sink_port));
			g.add(cc);
		}
		for (auto p: data->graph->out_ports) {
			int owner_id = data->graph->nodes.find(p->owner);
			int port_id = p->owner->out_ports.find(p);
			g.add(xml::Element("outport").witha("source", str(owner_id)).witha("source_port", str(port_id)));
		}
		for (auto p: data->graph->in_ports) {
			int owner_id = data->graph->nodes.find(p->owner);
			int port_id = p->owner->in_ports.find(p);
			g.add(xml::Element("inport").witha("sink", str(owner_id)).witha("sink_port", str(port_id)));
		}

		w.add(g);
	}

	p.save(filename);
}
