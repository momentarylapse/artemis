//
// Created by michi on 09.03.25.
//

#include "ModeDefault.h"

#include <lib/image/Painter.h>

#include "../view/MultiView.h"


ModeDefault::ModeDefault(Session* s) : Mode(s) {
	multi_view = new MultiView(session);
	//data = new DataModel(session);
	//generic_data = data.get();
}

void ModeDefault::on_draw_win(const RenderParams& params, MultiViewWindow* win) {

}

void ModeDefault::on_draw_post(Painter* p) {
	p->set_color(White);
	p->draw_str({100,100}, "Test");
}



