//
// Created by michi on 8/26/25.
//

#pragma once

#include "Canvas.h"
#include <lib/base/pointer.h>

namespace artemis::view {
	class LayoutGrid;
}

namespace artemis::graph {

class LayoutGrid : public RendererNode {
public:
	explicit LayoutGrid(Session* s);

	void on_process() override;

	dataflow::Setting<bool> horizontal{this, "horizontal", false};
	dataflow::Setting<int> spacing{this, "spacing", 0};
	dataflow::Setting<int> margin{this, "margin", 0};
	dataflow::InPort<RenderData> in_a{this, "a", dataflow::PortFlags::Optional};
	dataflow::InPort<RenderData> in_b{this, "b", dataflow::PortFlags::Optional};
	dataflow::InPort<RenderData> in_c{this, "c", dataflow::PortFlags::Optional};
	dataflow::InPort<RenderData> in_d{this, "d", dataflow::PortFlags::Optional};

	shared<view::LayoutGrid> layout_grid;
};

}
