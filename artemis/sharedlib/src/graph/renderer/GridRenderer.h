//
// Created by Michael Ankele on 2025-03-10.
//

#pragma once

#include "RendererNode.h"
#include <data/grid/Grid.h>
#include <lib/dataflow/Port.h>
#include <lib/dataflow/Setting.h>
#include <lib/image/color.h>

namespace artemis::graph {

class GridRenderer : public RenderEmitterNode {
public:
	explicit GridRenderer(Session* s) : RenderEmitterNode(s, "GridRenderer") {}

	void on_process() override;

	void on_emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) override;
	base::optional<Box> bounding_box() const override;

	enum class RenderMode {
		Outlines,
		Full
	};

	dataflow::Setting<double> line_width{this, "line-width", 1.0, "range=0:99:0.1"};
	dataflow::Setting<color> _color{this, "color", Gray};
	dataflow::SettingFromSet<RenderMode> mode{this, "mode", RenderMode::Full, {RenderMode::Outlines, RenderMode::Full}, {"Outlines", "Full"}};

	dataflow::InPort<data::Grid> in_grid{this, "grid"};
};

} // graph
