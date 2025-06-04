//
// Created by Michael Ankele on 2025-06-04.
//

#ifndef COLORMAPDIALOG_H
#define COLORMAPDIALOG_H

#include <data/util/ColorMap.h>
#include <lib/base/optional.h>
#include <lib/xhui/Dialog.h>


class ColorMapDialog : public xhui::Dialog {
public:
	ColorMapDialog(xhui::Panel* parent, const artemis::data::ColorMap& color_map);
	~ColorMapDialog() override;

	void on_draw(Painter* p);
	void on_left_button_down(const vec2& m) override;
	void on_left_button_up(const vec2& m) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;

	vec2 handle_pos(int index) const;
	base::optional<int> get_hover(const vec2& m) const;

	rect area;
	float value_min, value_max;
	float mouse_moved_since_click = 0;
	vec2 mouse_position_last = {0,0};
	base::optional<int> hover;
	base::optional<int> selected;

	base::promise<artemis::data::ColorMap> promise;
	artemis::data::ColorMap color_map;
	base::optional<artemis::data::ColorMap> answer;

	static base::future<artemis::data::ColorMap> ask(xhui::Panel* parent, const artemis::data::ColorMap& color_map);
};



#endif //COLORMAPDIALOG_H
