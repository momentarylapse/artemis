#include "Edit.h"
#include "../Painter.h"
#include "../Theme.h"
#include "../draw/font.h"
#include "../../base/iter.h"
#include "../../os/msg.h"

namespace xhui {

int next_utf8_index(const string& text, int index) {
	for (int i=index; i<min(index + 8, text.num); i++)
		if ((text[i] & 0x80) == 0x00 or (text[i] & 0xc0) == 0x80)
			return i + 1;
	return index;
}

int prior_utf8_index(const string& text, int index) {
	for (int i=max(index-1,0); i>=index-8; i--)
		if ((text[i] & 0x80) == 0x00 or (text[i] & 0xc0) == 0xc0)
			return i;
	return index;
}

Edit::Edit(const string &_id, const string &t) : Control(_id) {
	//state = State::DEFAULT;
	can_grab_focus = true;

	size_mode_x = SizeMode::Expand;
	size_mode_y = SizeMode::Shrink;

	font_name = Theme::_default.font_name;
	font_size = Theme::_default.font_size;

	tab_size = 4;
	face = nullptr;

	Edit::set_string(t);
}

void Edit::Cache::rebuild(const string& text) {
	*this = {};
	lines = text.explode("\n");
	if (lines.num == 0)
		lines.add("");
	int index = 0;
	for (const string& l: lines) {
		line_num_characters.add(l.num);
		line_first_index.add(index);
		index += l.num + 1;
	}
}

void Edit::set_string(const string &s) {
	text = s;
	cache.rebuild(text);
	set_cursor_pos(0);
	request_redraw();
}

void Edit::enable(bool _enabled) {
	enabled = _enabled;
	request_redraw();
}

string Edit::get_string() {
	return text;
}


vec2 Edit::get_content_min_size() const {
	return {80, 30};
}

void Edit::on_left_button_down(const vec2& m) {
	set_cursor_pos(xy_to_index(m));
}

void Edit::on_mouse_move(const vec2& m, const vec2& d) {
	if (owner->get_window()->button(0)) {
		set_cursor_pos(xy_to_index(m), true);
	}
}

void Edit::on_key_down(int key) {
	if (!enabled) {
		request_redraw();
		return;
	}
	const auto cur_lp = index_to_line_pos(cursor_pos);

	bool shift = (key & KEY_SHIFT);
	int key_no_shift = key & ~KEY_SHIFT;

	if (key_no_shift == KEY_LEFT)
		set_cursor_pos(clamp(prior_index(cursor_pos), 0, text.num), shift);
	if (key_no_shift == KEY_RIGHT)
		set_cursor_pos(clamp(next_index(cursor_pos), 0, text.num), shift);
#ifdef OS_MAC
	if (key_no_shift == KEY_LEFT + KEY_ALT)
#else
	if (key_no_shift == KEY_HOME)
#endif
		set_cursor_pos(cache.line_first_index[cur_lp.line], shift);
#ifdef OS_MAC
	if (key_no_shift == KEY_RIGHT + KEY_ALT)
#else
	if (key_no_shift == KEY_END)
#endif
		set_cursor_pos(cache.line_first_index[cur_lp.line] + cache.line_num_characters[cur_lp.line], shift);

	auto jump_lines = [this, cur_lp, shift] (int dlines) {
		set_cursor_pos(line_pos_to_index({cur_lp.line + dlines, cur_lp.offset}), shift);
	};
	if (key_no_shift == KEY_UP and multiline)
		jump_lines(-1);
	if (key_no_shift == KEY_DOWN and multiline)
		jump_lines(1);

#ifdef OS_MAC
	if (key == KEY_C + KEY_SUPER)
#else
	if (key == KEY_C + KEY_CONTROL)
#endif
		clipboard::copy(get_range(selection_start, cursor_pos));
#ifdef OS_MAC
	if (key == KEY_V + KEY_SUPER)
#else
	if (key == KEY_V + KEY_CONTROL)
#endif
		auto_insert(clipboard::paste());

	if (key == KEY_BACKSPACE) {
		if (cursor_pos != selection_start) {
			delete_selection();
		} else if (cursor_pos > 0) {
			delete_range(prior_index(cursor_pos), cursor_pos);
		}
	}
	if (key == KEY_DELETE) {
		if (cursor_pos != selection_start) {
			delete_selection();
		} else if (cursor_pos < text.num) {
			delete_range(cursor_pos, next_index(cursor_pos));
		}
	}

	if (key == KEY_KEY_CODE) {
		auto c = owner->get_window()->state.key_char;
		if (c != '\n' or multiline)
			auto_insert(utf32_to_utf8({c}));
	}
	if (key == KEY_RETURN and multiline)
		auto_insert("\n");
	if (key == KEY_TAB and multiline)
		auto_insert("\t");

	request_redraw();
}

void Edit::draw_active_marker(Painter* p) {

}

void Edit::draw_text(Painter* p) {
	const auto clip0 = p->clip();
	p->set_clip(_area);
	p->set_font(font_name, font_size, false, false);
	face = p->face;

	text_x0 = _area.x1 + Theme::_default.edit_margin_x;

	// update text dims
	float inner_height = 0;
	{
		auto& lines = cache.lines;
		cache.line_y0.clear();
		cache.line_height.clear();
		cache.line_width.clear();
		float y0 = _area.y1 + 8;
		for (const string &l: lines) {
			auto dim = face->get_text_dimensions(l);
			inner_height = dim.inner_height() / ui_scale;
			cache.line_height.add(dim.line_dy / ui_scale);
			cache.line_y0.add(y0);
			cache.line_width.add(dim.dx / ui_scale);
			y0 += dim.line_dy / ui_scale;
		}
		if (!multiline)
			cache.line_y0[0] = _area.center().y - inner_height / 2;
	}

	// selection
	if (cursor_pos != selection_start) {
		p->set_color(color(0.4f, 0.2f, 0.2f, 1.0f));
		auto a = cursor_pos;
		auto b = selection_start;
		if (a > b)
			std::swap(a, b);
		const auto lp0 = index_to_line_pos(a);
		const auto lp1 = index_to_line_pos(b);
		for (int l=lp0.line; l<=lp1.line; l++) {
			float x0 = (l == lp0.line) ? index_to_xy(a).x : text_x0;
			float x1 = (l == lp1.line) ? index_to_xy(b).x : text_x0 + cache.line_width[l];
			p->draw_rect({x0, x1, cache.line_y0[l], cache.line_y0[l] + cache.line_height[0]});
		}
		if (lp1.line == lp0.line) {
			const vec2 pos0 = index_to_xy(a);
			const vec2 pos1 = index_to_xy(b);
			p->draw_rect({pos0.x, pos1.x, pos0.y, pos0.y + cache.line_height[0]});
		}

	}


	// text
	p->set_color(Theme::_default.text_label);
	if (!enabled)
		p->set_color(Theme::_default.text_disabled);
	for (const auto& [line, l]: enumerate(cache.lines)) {
		p->draw_str({text_x0, cache.line_y0[line]}, l);
	}

	// cursor
	if (has_focus() and enabled) {
		const vec2 pos = index_to_xy(cursor_pos);
		p->draw_line({pos.x, pos.y - 3}, {pos.x, pos.y + font_size + 3});
	}
	p->set_clip(clip0);
}

string Edit::get_range(Index _i0, Index _i1) const {
	auto i0 = min(_i0, _i1);
	auto i1 = max(_i0, _i1);
	return text.sub_ref(i0, i1);
}

void Edit::delete_range(Index i0, Index i1) {
	replace_range(i0, i1, "");
}

void Edit::delete_selection() {
	delete_range(selection_start, cursor_pos);
}

void Edit::replace_range(Index _i0, Index _i1, const string& t) {
	auto i0 = min(_i0, _i1);
	auto i1 = max(_i0, _i1);
	text = text.sub_ref(0, i0) + t + text.sub_ref(i1);
	cache.rebuild(text);
	if (cursor_pos >= i1)
		set_cursor_pos(cursor_pos - (i1 - i0) + t.num);
	else if (cursor_pos >= i0)
		set_cursor_pos(i0 + t.num);
	on_edit();
	emit_event(event_id::Changed, true);
}

void Edit::auto_insert(const string& t) {
	replace_range(selection_start, cursor_pos, t);
}

void Edit::set_cursor_pos(Index index, bool selecting) {
	cursor_pos = index;
	if (!selecting)
		selection_start = index;
	request_redraw();
}


vec2 Edit::index_to_xy(Index index) const {
	auto lp = index_to_line_pos(index);
	int first = cache.line_first_index[lp.line];
	face->set_size(font_size * ui_scale);
	auto dim = face->get_text_dimensions(text.sub_ref(first, index));
	float x = text_x0 + dim.bounding_width / ui_scale;
	float y0 = cache.line_y0[lp.line];
	return {x, y0};
}

Edit::Index Edit::xy_to_index(const vec2& pos) const {
	float dx = pos.x - text_x0;
	float dy = pos.y - cache.line_y0[0];

	face->set_size(font_size * ui_scale);
	int line_no = min((int)(dy / cache.line_height[0]), cache.lines.num - 1);
	const auto& l = cache.lines[line_no];

	if (dx > cache.line_width[line_no])
		return line_pos_to_index({line_no, cache.line_num_characters[line_no]});

	int best_i = 0;
	float best_dx = 10000000;
	for (int i=0; i<l.num; i=next_utf8_index(l, i)) {
		auto dim = face->get_text_dimensions(l.sub_ref(0, i));
		float ddx = fabs(dim.bounding_width / ui_scale - dx);
		if (ddx < best_dx) {
			best_dx = ddx;
			best_i = i;
		}
	}
	return line_pos_to_index({line_no, best_i});
}



void Edit::_draw(Painter *p) {

	// background
	color bg = Theme::_default.background_button;
	p->set_color(bg);
	p->set_roundness(Theme::_default.button_radius);
	p->draw_rect(_area);

	// focus frame
	if (has_focus() and show_focus_frame) {
		p->set_color(Theme::_default.background_button_primary.with_alpha(0.6f));
		p->draw_rect(_area);

		float dr = Theme::_default.focus_frame_width;
		p->set_roundness(Theme::_default.button_radius - dr);
		p->set_color(bg);
		p->draw_rect(_area.grow(-dr));
	}
	p->set_line_width(1);
	p->set_roundness(0);

	draw_text(p);
}

// TODO count utf8 chars
Edit::LinePos Edit::index_to_line_pos(Index index) const {
	LinePos r = {0, 0};
	for (const auto& [line, first]: enumerate(cache.line_first_index)) {
		if (index < first)
			break;
		r = {line, index - first};
	}
	return r;
}

// TODO count utf8 chars
Edit::Index Edit::line_pos_to_index(const LinePos& lp) const {
	if (lp.line < 0)
		return 0;
	if (lp.line >= cache.lines.num)
		return text.num;
	return cache.line_first_index[lp.line] + clamp(lp.offset, 0, cache.line_num_characters[lp.line]);
}

Edit::Index Edit::next_index(Index index) const {
	return next_utf8_index(text, index);
}

Edit::Index Edit::prior_index(Index index) const {
	return prior_utf8_index(text, index);
}


void Edit::set_option(const string& key, const string& value) {
	if (key == "focusframe") {
		show_focus_frame = value._bool();
	} else if (key == "font") {
		font_name = value;
	} else if (key == "fontsize") {
		font_size = value._float();
	} else {
		Control::set_option(key, value);
	}
}


}
