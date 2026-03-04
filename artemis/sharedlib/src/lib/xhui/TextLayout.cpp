#include "TextLayout.h"
#include "Theme.h"
#include "xhui.h"

namespace xhui {



TextLayout TextLayout::from_format_string(::Painter* p, const string& s, float font_size) {
	if (font_size < 0)
		font_size = xhui::Theme::_default.font_size;
	vec2 pos = vec2(0, 0);
	TextLayout l;
	auto add = [&] (const string& text, bool bold) {
		auto face = xhui::pick_font(xhui::Theme::_default.font_name, bold, false);
		face->set_size(font_size * p->ui_scale);
		l.parts.add({text, font_size, base::None, bold, false, pos});

		const auto dims = face->get_text_dimensions(text);
		vec2 size = {dims.bounding_width / p->ui_scale, dims.inner_height() / p->ui_scale};
		l.parts.back().box = {pos.x, pos.x + size.x, pos.y, pos.y + size.y};
		pos += vec2(size.x, 0);
	};

	// TODO better format parsing...
	int offset = 0;
	while (true) {
		int next = s.find("<b>", offset);
		if (next >= offset) {
			if (next > offset)
				add(s.sub_ref(offset, next), false);
			offset = next + 3;
			next = s.find("</b>", offset);
			if (next > offset) {
				add(s.sub_ref(offset, next), true);
				offset = next + 4;
			}
		} else {
			add(s.sub_ref(offset), false);
			break;
		}
	}
	return l;
}

rect TextLayout::box() const {
	rect r = rect::EMPTY;
	for (const auto& t: parts)
		r = r or t.box;
	return r;
}


void draw_text_layout(::Painter* p, const vec2& pos, const TextLayout& l) {
	for (const auto& t: l.parts) {
		if (t.col)
			p->set_color(*t.col);
		p->set_font("", t.font_size, t.bold, t.italic);
		p->draw_str(pos + t.pos, t.text);
	}
}

void draw_text_layout_with_box(::Painter* p, const vec2& pos, const TextLayout& l, const color& bg, float padding, float roundness) {
	/*vec2 size = p->get_str_size(str);
	vec2 pos = _pos;
	if (align == 0)
		pos.x -= size.x / 2;
	if (align == 1)
		pos.x -= size.x;*/
	p->set_color(bg);
	p->set_roundness(roundness);
	auto box = l.box();
	p->draw_rect(rect(pos + box.p00(), pos + box.p11()).grow(padding));
	p->set_color(xhui::Theme::_default.text_label);
	p->set_roundness(0);
	draw_text_layout(p, pos, l);

	//draw_boxed_str(p, pos, l.parts[0].text);
}

}
