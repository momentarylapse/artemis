//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include <lib/base/optional.h>
#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <lib/math/Box.h>
#include <lib/ygraphics/graphics-fwd.h>

namespace yrenderer {
	struct RenderViewData;
	struct RenderParams;
}
class MultiViewWindow;
class Session;
class Painter;

namespace artemis::graph {

struct RenderData {
	// TODO
	bool active;
	base::optional<Box> bounding_box;
	std::function<void(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd)> f_draw_3d;
	std::function<void(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd)> f_draw_3d_transparent;
	std::function<void(Painter*)> f_draw_2d;
};

class RendererNode : public dataflow::Node {
public:
	explicit RendererNode(Session* s, const string& name);

	// TODO send RenderData to Canvas instead!
	//virtual void draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd) = 0;

	dataflow::Setting<bool> active{this, "active", true};

	dataflow::OutPort<RenderData> out_draw{this, "draw"};

	Session* session;
};

} // graph

