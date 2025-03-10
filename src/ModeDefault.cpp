//
// Created by michi on 09.03.25.
//

#include "ModeDefault.h"

#include <Session.h>
#include <graph/Graph.h>
#include <graph/Node.h>
#include <graph/NodeFactory.h>
#include <graph/Port.h>
#include <graph/renderer/MeshRenderer.h>
#include <lib/image/Painter.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <lib/os/msg.h>
#include <view/ArtemisWindow.h>
#include <view/DrawingHelper.h>
#include <view/MultiView.h>


ModeDefault::ModeDefault(Session* s) : Mode(s) {
	multi_view = new MultiView(session);
	//data = new DataModel(session);
	//generic_data = data.get();

	graph = session->graph.get();

	xhui::run_repeated(1.0f, [this] {
		graph->iterate();
		session->win->request_redraw();
	});
}

void ModeDefault::on_draw_win(const RenderParams& params, MultiViewWindow* win) {
	for (auto n: graph->nodes)
		if (auto r = dynamic_cast<graph::MeshRenderer*>(n)) {
			auto vb = r->vertex_buffer.get();
			if (!vb)
				continue;

			session->drawing_helper->draw_mesh(params, win->rvd, mat4::ID, vb, r->material.get());
		}
}

void ModeDefault::on_draw_post(Painter* p) {
}

void ModeDefault::on_key_down(int key) {
	if (key == xhui::KEY_S + xhui::KEY_CONTROL) {

	}
}




