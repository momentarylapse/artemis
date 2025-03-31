//
// Created by michi on 09.03.25.
//

#ifndef MODEDEFAULT_H
#define MODEDEFAULT_H

#include "view/Mode.h"
#include <lib/base/pointer.h>

namespace artemis::graph {
	class Graph;
}


class ModeDefault : public Mode {
public:
	explicit ModeDefault(Session* session);

	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_key_down(int key) override;

	artemis::graph::Graph* graph;

	bool simulation_active = false;

	void update_menu();
};



#endif //MODEDEFAULT_H
