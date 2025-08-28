//
// Created by michi on 8/28/25.
//

#pragma once

#pragma once

#include "Canvas.h"
#include <lib/base/pointer.h>

namespace artemis::view {
	class LayoutOverlay;
}

namespace artemis::graph {

class LayoutOverlay : public RendererNode {
public:
	explicit LayoutOverlay(Session* s);

	void on_process() override;

	dataflow::InPort<RenderData> in_a{this, "a", dataflow::PortFlags::Optional};
	dataflow::InPort<RenderData> in_b{this, "b", dataflow::PortFlags::Optional};
	dataflow::InPort<RenderData> in_c{this, "c", dataflow::PortFlags::Optional};

	shared<view::LayoutOverlay> layout_overlay;
};

}

