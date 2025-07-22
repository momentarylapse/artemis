//
// Created by Michael Ankele on 2025-03-16.
//

#include "Plotter.h"
#include <Session.h>
#include <lib/os/msg.h>
#include <lib/image/Painter.h>
#include <lib/math/rect.h>
#include <lib/xhui/Theme.h>

namespace artemis::graph {

void Plotter::on_process() {
	RenderData d;
	d.f_draw_2d = [this] (Painter* p) {
		draw_2d(p);
	};
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
	float dx = pow(10.0f, round(log10(40 / scale)));
	x_min = floor(x_min / dx) * dx;
	for (float x = x_min; x < x_max; x+=dx)
		xs.add(x);
	return xs;
}

void Plotter::draw_2d(Painter* p) {
	auto area = p->area();

	float y_min = -5;
	float y_max = 15;

	float x_scale = area.width() / (x_max() - x_min());
	float x_offset = area.x1 - x_min() * x_scale;

	float y_scale = - area.height() / (y_max - y_min);
	float y_offset = area.y1 - y_max * y_scale;

	auto project = [=] (const vec2& v) {
		return vec2(x_offset + v.x * x_scale, y_offset + v.y * y_scale);
	};

	p->set_color(color::interpolate(xhui::Theme::_default.text_disabled, xhui::Theme::_default.background_low, 0.75f));
	p->set_line_width(1);
	for (float x: ticks(x_min(), x_max(), x_scale))
		p->draw_line(project({x, y_min}), project({x, y_max}));
	for (float y: ticks(y_min, y_max, -y_scale))
		p->draw_line(project({x_min(), y}), project({x_max(), y}));

	for (const auto d: in_plot.values()) {
		Array<vec2> points;

		if (d->f) {
			Array<float> xs = lin_spacing(x_min(), x_max(), 200), ys;
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
		p->set_line_width(d->line_width);
		p->draw_lines(points);
	}
}


} // graph
