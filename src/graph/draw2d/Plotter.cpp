//
// Created by Michael Ankele on 2025-03-16.
//

#include "Plotter.h"
#include <Session.h>
#include <view/Canvas.h>
#include <lib/image/Painter.h>
#include <lib/math/rect.h>
#include <lib/xhui/Theme.h>

#include "lib/os/msg.h"
#include "lib/xhui/xhui.h"

namespace artemis::graph {

class PlotterRenderNode : public view::RenderNode {
public:
	Plotter* plotter;
	explicit PlotterRenderNode(Plotter* _plotter) : RenderNode(_plotter->session) {
		add_control("DrawingArea", "", 0, 0, "area");
		plotter = _plotter;
		event_xp("area", xhui::event_id::Draw, [this] (Painter* p) {
			plotter->draw_2d(p);
		});
	}
};

Plotter::Plotter(Session* s) : RendererNode(s, "Plotter") {
	render_node = new PlotterRenderNode(this);
}

void Plotter::on_process() {
	RenderData d;
	d.render_node = render_node.get();
	out_draw(d);
}

Array<float> lin_spacing(float x_min, float x_max, int N) {
	Array<float> xs;
	for (int i=0; i<N; i++) {
		float x = x_min + ((x_max - x_min) / (float)N) * (float)i;
		xs.add(x);
	}
	return xs;
}

Array<float> ticks(float x_min, float x_max, float scale) {
	Array<float> xs;
	float dx = powf(10.0f, roundf(log10f(40 / scale)));
	x_min = floorf(x_min / dx) * dx;
	for (float x = x_min; x < x_max; x+=dx)
		xs.add(x);
	return xs;
}

void Plotter::draw_2d(Painter* p) {
	auto area = p->area();

	float x_scale = area.width() / (float)(x_max() - x_min());
	float x_offset = area.x1 - (float)x_min() * x_scale;

	if (auto_zoom()) {
		float y0 = 0, y1 = 0;
		for (const auto d: in_plot.values()) {
			if (d->f) {

			} else {
				if (y0 == y1 and d->points.num > 0)
					y0 = y1 = d->points[0].y;
				for (const auto& pp: d->points) {
					y0 = min(y0, pp.y);
					y1 = max(y1, pp.y);
				}
			}
		}
		if (y0 != y1) {
			y_min.set(y0 - (y1-y0) * 0.1f);
			y_max.set(y1 + (y1-y0) * 0.1f);
			state = dataflow::NodeState::Complete; // don't trigger updates
		}
	}

	float y_scale = - area.height() / ((float)y_max() - (float)y_min());
	float y_offset = area.y1 - (float)y_max() * y_scale;

	auto project = [=] (const vec2& v) {
		return vec2(x_offset + v.x * x_scale, y_offset + v.y * y_scale);
	};

	p->set_color(color::interpolate(xhui::Theme::_default.text_disabled, xhui::Theme::_default.background_low, 0.75f));
	p->set_line_width(1);
	for (float x: ticks((float)x_min(), (float)x_max(), x_scale))
		p->draw_line(project({x, (float)y_min()}), project({x, (float)y_max()}));
	for (float y: ticks((float)y_min(), (float)y_max(), -y_scale))
		p->draw_line(project({(float)x_min(), y}), project({(float)x_max(), y}));

	for (const auto d: in_plot.values()) {
		Array<vec2> points;

		if (d->f) {
			Array<float> xs = lin_spacing((float)x_min(), (float)x_max(), 200), ys;
			for (float x: xs) {
				float y = (*d->f)(x);
				ys.add(y);
			}

			for (int i=0; i<xs.num; i++)
				points.add(project({xs[i], ys[i]}));
		} else {
			for (const vec2& v: d->points)
				points.add(project(v));
		}

		p->set_color(d->_color);
		p->set_line_width((float)d->line_width);
		p->draw_lines(points);
	}
}


} // graph
