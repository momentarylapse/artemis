//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef RENDERERNODE_H
#define RENDERERNODE_H

#include <graph/Port.h>
#include <graph/Setting.h>

#include "../Node.h"
#include <y/graphics-fwd.h>

class MultiViewWindow;
struct RenderParams;
class Session;

namespace graph {

struct RenderData {
	// TODO
	int dummy;
};

class RendererNode : public Node {
public:
	explicit RendererNode(Session* s, const string& name);

	// TODO send RenderData to Canvas instead!
	virtual void draw_win(const RenderParams& params, MultiViewWindow* win) = 0;

	Setting<bool> active{this, "active", true};

	OutPort<RenderData> out_draw{this, "draw"};

	Session* session;
};

} // graph

#endif //RENDERERNODE_H
