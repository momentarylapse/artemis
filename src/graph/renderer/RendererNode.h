//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include <lib/base/optional.h>
#include <lib/base/pointer.h>
#include <lib/dataflow/Node.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <lib/math/Box.h>
//#include <lib/ygraphics/graphics-fwd.h>

namespace yrenderer {
	struct RenderViewData;
	struct RenderParams;
	class MeshEmitter;
}
class MultiViewWindow;
class Session;
class Painter;

namespace artemis::view {
	class RenderNode;
}

namespace artemis::graph {

struct DrawCall {
	bool active;
	base::optional<Box> bounding_box;
	yrenderer::MeshEmitter* emitter = nullptr;
	bool transparent = false;
};

struct RenderData {
	// TODO
	bool active;
	base::optional<Box> bounding_box;
	std::function<void(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd)> f_draw_3d;
	std::function<void(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd)> f_draw_3d_transparent;
	view::RenderNode* render_node = nullptr;
};

class RenderEmitterNode : public dataflow::Node {
public:
	explicit RenderEmitterNode(Session* s, const string& name);
	~RenderEmitterNode() override;

	virtual void on_emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) {}
	virtual base::optional<Box> bounding_box() const { return base::None; }
	void send_out(bool transparent = false);

	dataflow::Setting<bool> active{this, "active", true};

	dataflow::OutPort<DrawCall> out_draw{this, "draw"};

	Session* session;

	class Emitter;
	shared<Emitter> emitter;
};

class RendererNode : public dataflow::Node {
public:
	explicit RendererNode(Session* s, const string& name);

	dataflow::OutPort<RenderData> out_draw{this, "draw"};

	Session* session;
};

} // graph

