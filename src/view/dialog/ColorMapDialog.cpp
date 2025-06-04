//
// Created by Michael Ankele on 2025-06-04.
//

#include "ColorMapDialog.h"
#include <lib/xhui/xhui.h>
#include <lib/xhui/dialogs/ColorSelectionDialog.h>

ColorMapDialog::ColorMapDialog(xhui::Panel* parent, const artemis::data::ColorMap& _color_map) : Dialog("Question", 400, 300, parent, xhui::DialogFlags::CloseByEscape) {
	from_source(R"foodelim(
Dialog color-map-dialog 'Color map'
	Grid ? ''
		DrawingArea area '' expandx expandy
		---|
		Grid ? ''
			Button ok 'Ok'
			Button cancel 'Cancel'
)foodelim");
	color_map = _color_map;
	value_min = color_map.min();
	value_max = color_map.max();
	event_xp("area", xhui::event_id::Draw, [this] (Painter* p) {
		on_draw(p);
	});
	event_x("area", xhui::event_id::MouseMove, [this] {
		const vec2 m = get_window()->mouse_position();
		on_mouse_move(m, m - mouse_position_last);
		mouse_position_last = m;
	});
	event_x("area", xhui::event_id::LeftButtonDown, [this] {
		on_left_button_down(get_window()->mouse_position());
	});
	event_x("area", xhui::event_id::LeftButtonUp, [this] {
		on_left_button_up(get_window()->mouse_position());
	});
	event("ok", [this] {
		answer = color_map;
		request_destroy();
	});
	event("cancel", [this] {
		request_destroy();
	});
	event(xhui::event_id::Close, [this] {
		request_destroy();
	});
}

ColorMapDialog::~ColorMapDialog() = default;


void ColorMapDialog::on_draw(Painter* p) {
	auto clip = p->clip();
	area = p->area();
	p->set_clip(area);

	const float d = 20;
	for (int i=0; i<=area.width()/d; i++)
		for (int j=0; j<=area.height()/d; j++) {
			p->set_color(((i+j) % 2) == 0 ? Black : White);
			p->draw_rect({area.p00() + vec2((float)i*d, (float)j*d), area.p00() + vec2((float)(i+1)*d, (float)(j+1)*d)});
		}

	const int N = 200;
	for (int k=0; k<N; k++) {
		float t0 = (float)k / (float)N;
		float t1 = (float)(k+1) / (float)N;
		float t = value_min + (value_max - value_min) * t0;
		p->set_color(color_map.get(t));
		p->draw_rect(rect(area.x1 + area.width() * t0, area.x1 + area.width() * t1, area.y1, area.y2));
	}

	p->set_clip(clip);

	for (int i=0; i<color_map.colors.num; i++) {
		if (hover and *hover == i)
			p->set_color(Gray);
		else
			p->set_color(Black);
		p->draw_circle(handle_pos(i), 10);
		p->set_color(White);
		p->draw_circle(handle_pos(i), 7);
	}
}

vec2 ColorMapDialog::handle_pos(int index) const {
	float t0 = (color_map.values[index] - value_min) / (value_max - value_min);
	return area.p00() + vec2(area.width() * t0, area.height() * (1 - color_map.colors[index].a));
}


base::optional<int> ColorMapDialog::get_hover(const vec2& m) const {
	for (int i=0; i<color_map.colors.num; i++)
		if ((handle_pos(i) - m).length() < 20)
			return i;
	return base::None;
}


void ColorMapDialog::on_mouse_move(const vec2& m, const vec2& d) {
	if (get_window()->button(0) and selected) {
		mouse_moved_since_click += d.length();
		color_map.values[*selected] = value_min + (m.x - area.x1) / area.width() * (value_max - value_min);
		color_map.colors[*selected].a = clamp(1 - (m.y - area.y1) / area.height(), 0.0f, 1.0f);
	} else {
		hover = get_hover(m);
	}
	request_redraw();
}

void ColorMapDialog::on_left_button_down(const vec2& m) {
	hover = get_hover(m);
	selected = hover;
	mouse_moved_since_click = 0;
}

void ColorMapDialog::on_left_button_up(const vec2& m) {
	value_min = color_map.min();
	value_max = color_map.max();
	if (mouse_moved_since_click < 10 and selected) {
		xhui::ColorSelectionDialog::ask(this, "Color", color_map.colors[*selected].with_alpha(1), {}).then([this] (const color& c) {
			color_map.colors[*selected] = c.with_alpha(color_map.colors[*selected].a);
			request_redraw();
		});
	}
	request_redraw();
}


base::future<artemis::data::ColorMap> ColorMapDialog::ask(xhui::Panel* parent, const artemis::data::ColorMap& color_map) {
	auto dlg = new ColorMapDialog(parent, color_map);
	parent->open_dialog(dlg).then([dlg] {
		if (dlg->answer)
			dlg->promise(*dlg->answer);
		else
			dlg->promise.fail();
	});
	return dlg->promise.get_future();
}

