//
// Created by michi on 8/27/25.
//

#pragma once

#include "RendererNode.h"
#include "../canvas/Canvas.h"
#include <lib/base/pointer.h>
#include <lib/image/color.h>

namespace artemis::view {
	class SceneRenderer;
}

namespace artemis::graph {

class SceneRenderer : public RendererNode {
public:
	explicit SceneRenderer(Session* s);

	void additional_init() override;
	void on_process() override;

	dataflow::Setting<color> background{this, "background", color(-1,-1,-1,-1)};

	dataflow::InPort<DrawCall> in_draw{this, "draw", dataflow::PortFlags::Multi};

	shared<view::SceneRenderer> scene_renderer;
};

}
