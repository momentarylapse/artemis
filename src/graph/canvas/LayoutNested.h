//
// Created by michi on 8/28/25.
//

#pragma once

#include "Canvas.h"
#include <lib/base/pointer.h>

namespace artemis::view {
	class LayoutNested;
}

namespace artemis::graph {

class LayoutNested : public RendererNode {
public:
	explicit LayoutNested(Session* s);

	void on_process() override;

	dataflow::Setting<int> mode{this, "mode", 0};
	dataflow::InPort<RenderData> in_main{this, "main", dataflow::PortFlags::Optional};
	dataflow::InPort<RenderData> in_a{this, "a", dataflow::PortFlags::Optional};
	dataflow::InPort<RenderData> in_b{this, "b", dataflow::PortFlags::Optional};
	dataflow::InPort<RenderData> in_c{this, "c", dataflow::PortFlags::Optional};

	shared<view::LayoutNested> layout_nested;
};

}

