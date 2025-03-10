//
// Created by Michael Ankele on 2025-03-10.
//

#ifndef RENDERERNODE_H
#define RENDERERNODE_H

#include "../Node.h"
#include <y/graphics-fwd.h>

class MultiViewWindow;
struct RenderParams;
class Session;

namespace graph {

class RendererNode : public Node {
public:
	explicit RendererNode(Session* s, const string& name);
	virtual void draw_win(const RenderParams& params, MultiViewWindow* win) = 0;

	Session* session;
};

} // graph

#endif //RENDERERNODE_H
