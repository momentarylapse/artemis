//
// Created by michi on 8/26/25.
//

#include "SceneRenderer.h"
#include "ArtemisWindow.h"
#include "../Session.h"
#include <lib/yrenderer/scene/path/RenderPathForward.h>
#include <lib/yrenderer/target/XhuiRenderer.h>
#include <lib/math/Box.h>
#include <lib/xhui/xhui.h>

#include "DrawingHelper.h"

namespace artemis::view {

rect scale_area(const rect& area, float scale) {
	return {area.p00() * scale, area.p11() * scale};
}


yrenderer::CameraParams ViewPort::params() const {
	return {pos - ang * vec3(0,0,radius), ang, fov, radius * 0.01f, radius * 100.0f};
}

void ViewPort::move(const vec3& dpos) {
	pos += ang * (dpos * radius);
	defined_by_user = true;
}

void ViewPort::rotate(const quaternion& dq) {
	ang = ang * dq;
	defined_by_user = true;
}

void ViewPort::zoom(float factor) {
	radius *= factor;
	defined_by_user = true;
}


void ViewPort::focus_on_box(const Box& box) {
	pos = box.center();
	radius = box.size().length() * 2;
	defined_by_user = true;
}



SceneRenderer::SceneRenderer(Session* s) : RenderNode(s) {
	session = s;

	from_source(R"foodelim(
Dialog x x padding=0
		Overlay ? ''
			DrawingArea area '' grabfocus
			Grid overlay-main-grid '' margin=25
				Grid ? ''
					Label ? '' ignorehover expandx
					Grid overlay-button-grid-right '' spacing=20
						Button cam-rotate 'R' image=rf-rotate height=50 width=50 padding=7 noexpandx ignorefocus
						---|
						Button cam-move 'M' image=rf-translate height=50 width=50 padding=7 noexpandx ignorefocus
				---|
				Label ? '' ignorehover expandy
)foodelim");

	view_port.pos = {0,0,0};
	view_port.ang = quaternion::ID;
	view_port.radius = 5;
	view_port.fov = pi / 4;

	lights.resize(1);
	lights[0].init(White, -1, -1);
	lights[0]._ang = quaternion::ID;


	session->win->event_xp(session->win->id, xhui::event_id::JustBeforeDraw, [this] (Painter* p) {
		auto pp = static_cast<xhui::Painter*>(p);
		const auto a0 = pp->area();
		const auto na0 = pp->native_area;
		pp->_area = _area;
		pp->native_area = scale_area(_area, pp->ui_scale);
		on_pre_draw(p);
		pp->_area = a0;
		pp->native_area = na0;
	});

	event_xp("area", xhui::event_id::Draw, [this] (Painter* p) {
		on_draw(p);
	});

	/*panel->event_x(id, xhui::event_id::MouseMove, [this] {
		handle_event({Event::Type::MouseMove, panel->get_window()->mouse_position(), {0,0}, {0,0}, 0});
	});
	panel->event_x(id, xhui::event_id::LeftButtonDown, [this] {
		handle_event({Event::Type::LeftMouseDown, panel->get_window()->mouse_position(), {0,0}, {0,0}, 0});
	});
	panel->event_x(id, xhui::event_id::LeftButtonUp, [this] {
		handle_event({Event::Type::LeftMouseUp, panel->get_window()->mouse_position(), {0,0}, {0,0}, 0});
	});
	panel->event_x(id, xhui::event_id::MiddleButtonDown, [this] {
		handle_event({Event::Type::MiddleMouseUp, panel->get_window()->mouse_position(), {0,0}, {0,0}, 0});
	});
	panel->event_x(id, xhui::event_id::RightButtonDown, [this] {
		handle_event({Event::Type::RightMouseUp, panel->get_window()->mouse_position(), {0,0}, {0,0}, 0});
	});
	panel->event_x(id, xhui::event_id::RightButtonDown, [this] {
		handle_event({Event::Type::RightMouseUp, panel->get_window()->mouse_position(), {0,0}, {0,0}, 0});
	});*/
	event_x("area", xhui::event_id::MouseWheel, [this] {
		view_port.zoom(expf(-0.1f * get_window()->state.scroll.y));
		request_redraw();
	});

	event_x("cam-move", xhui::event_id::LeftButtonDown, [this] {
		get_window()->set_mouse_mode(0);
	});
	event_x("cam-move", xhui::event_id::LeftButtonUp, [this] {
		get_window()->set_mouse_mode(1);
	});
	event_x("cam-move", xhui::event_id::MouseMove, [this] {
		if (auto w = get_window()) {
			vec2 d = w->state.m - w->state_prev.m;
			if (w->state.lbut) {
					if (w->is_key_pressed(xhui::KEY_SHIFT))
						view_port.move(vec3(0,0,d.y) / 800.0f);
					else
						view_port.move(vec3(-d.x, d.y, 0) / 800.0f);
					request_redraw();
			}
		}
	});
	event_x("cam-rotate", xhui::event_id::LeftButtonDown, [this] {
		get_window()->set_mouse_mode(0);
	});
	event_x("cam-rotate", xhui::event_id::LeftButtonUp, [this] {
		get_window()->set_mouse_mode(1);
	});
	event_x("cam-rotate", xhui::event_id::MouseMove, [this] {
		if (auto w = get_window()) {
			vec2 d = w->state.m - w->state_prev.m;
			if (w->state.lbut) {
				view_port.rotate(quaternion::rotation({d.y*0.003f, d.x*0.003f, 0}));
				request_redraw();
			}
		}
	});
}

SceneRenderer::~SceneRenderer() = default;

bool SceneRenderer::build() {
	if (!session->ctx)
		return false;

	if (!render_target) {
		render_target = new yrenderer::XhuiRenderer(session->ctx);
		render_path = new yrenderer::RenderPathForward(session->ctx, 2048);
		render_target->add_child(render_path.get());
	}
	return true;
}

void SceneRenderer::set_content_bounding_box(const Box &b) {
	if (!view_port.defined_by_user)
		view_port.focus_on_box(b);
}


void SceneRenderer::set_drawing_helper() {
	session->drawing_helper->target_area = _area;
	session->drawing_helper->projection = view_port.params().projection_matrix(_area.width() / _area.height());
	session->drawing_helper->view = view_port.params().view_matrix();
}


void SceneRenderer::on_pre_draw(Painter *p) {
	if (render_target) {
		render_path->set_view(view_port.params());
		lights[0]._ang = view_port.ang;
		render_path->set_lights({&lights[0]});

		set_drawing_helper();

		render_target->before_draw(p);
	}
}

void SceneRenderer::on_draw(Painter *p) {
	if (render_target) {
		set_drawing_helper();

		render_target->draw(p);
	}
}




}
