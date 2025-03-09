//
// Created by michi on 09.03.25.
//

#ifndef MODEDEFAULT_H
#define MODEDEFAULT_H

#include "view/Mode.h"


class ModeDefault : public Mode {
public:
	explicit ModeDefault(Session* session);

	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;

};



#endif //MODEDEFAULT_H
