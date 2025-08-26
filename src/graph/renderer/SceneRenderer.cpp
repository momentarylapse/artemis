//
// Created by michi on 8/27/25.
//

#include "SceneRenderer.h"
#include <view/SceneRenderer.h>
#include <lib/xhui/Theme.h>
#include <lib/yrenderer/scene/path/RenderPath.h>

namespace artemis::graph {

SceneRenderer::SceneRenderer(Session *s) : RendererNode(s, "SceneRenderer") {
	scene_renderer = new view::SceneRenderer(session);
	state = dataflow::NodeState::Uninitialized;
	background.set(xhui::Theme::_default.background_low);
}

void SceneRenderer::additional_init() {
	if (scene_renderer->build()) {
		state = dataflow::NodeState::Dirty;
	}
}


void SceneRenderer::on_process() {
	scene_renderer->render_path->remove_all_emitters();
	scene_renderer->render_path->background_color = background();
	for (auto& x: in_draw.values()) {
		if (x->emitter and x->active) {
			if (x->transparent)
				scene_renderer->render_path->add_transparent_emitter(x->emitter);
			else
				scene_renderer->render_path->add_opaque_emitter(x->emitter);
		}
	}

	RenderData d;
	d.render_node = scene_renderer.get();
	out_draw(d);
}



}
