//
// Created by Michael Ankele on 2025-03-10.
//

#include "RendererNode.h"
#include <lib/yrenderer/scene/MeshEmitter.h>
#include <lib/os/msg.h>
#include <view/DrawingHelper.h>
#include "Session.h"

namespace artemis {
	extern Session* default_session;
}

namespace artemis::graph {

class RenderEmitterNode::Emitter : public yrenderer::MeshEmitter {
public:
	RenderEmitterNode* owner;
	explicit Emitter(yrenderer::Context* ctx, RenderEmitterNode* o) : MeshEmitter(ctx, "em") {
		if (!ctx)
			msg_error("new Emitter: no context...");
		owner = o;
	}
	void emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) override {
		owner->on_emit(params, rvd, shadow_pass);
	}
};

RenderEmitterNode::RenderEmitterNode(const string& name) : Node(name) {
	session = default_session;
	flags = dataflow::NodeFlags::Renderer;
	if (session)
		emitter = new Emitter(session->ctx, this);
}

RenderEmitterNode::~RenderEmitterNode() = default;

void RenderEmitterNode::send_out(bool transparent) {
	DrawCall c;
	c.active = active();
	c.emitter = emitter.get();
	c.transparent = transparent;
	c.bounding_box = bounding_box();
	out_draw(c);
}

void RenderEmitterNode::draw_mesh(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd,
								  const mat4& matrix, ygfx::VertexBuffer* vb, yrenderer::Material* material) {
	session->drawing_helper->draw_mesh(params, rvd, matrix, vb, material);
}

} // graph