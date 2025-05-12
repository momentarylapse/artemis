//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef RENDERERNODE_H
#define RENDERERNODE_H

#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <lib/math/Box.h>
#include <y/graphics-fwd.h>

struct RenderViewData;
class MultiViewWindow;
struct RenderParams;
class Session;
class Painter;

namespace artemis::graph {

struct RenderData {
	// TODO
	base::optional<Box> bounding_box;
	std::function<void(Painter*)> f_draw_2d;
};

class RendererNode : public dataflow::Node {
public:
	explicit RendererNode(Session* s, const string& name);

	// TODO send RenderData to Canvas instead!
	virtual void draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd) = 0;

	dataflow::Setting<bool> active{this, "active", true};

	dataflow::OutPort<RenderData> out_draw{this, "draw"};

	Session* session;
};

} // graph

#endif //RENDERERNODE_H
