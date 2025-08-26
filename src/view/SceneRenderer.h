//
// Created by michi on 8/26/25.
//

#pragma once

#include "Canvas.h"
#include <lib/base/pointer.h>
#include <lib/yrenderer/scene/SceneRenderer.h>

namespace yrenderer {
	class RenderPath;
	class XhuiRenderer;
}

struct Box;
class Session;

namespace artemis::view {

struct ViewPort {
	vec3 pos;
	quaternion ang;
	float radius;
	float fov;

	void move(const vec3& dpos);
	void rotate(const quaternion& dq);
	void focus_on_box(const Box& box);

	yrenderer::CameraParams params() const;
};

class SceneRenderer : public RenderNode {
public:
	explicit SceneRenderer(Session *s);
	~SceneRenderer() override;

	bool build();

	void on_pre_draw(Painter *p);
	void on_draw(Painter *p);

	void set_drawing_helper();

	Session* session;
	owned<yrenderer::XhuiRenderer> render_target;
	owned<yrenderer::RenderPath> render_path;
	Array<yrenderer::Light> lights;

	ViewPort view_port;
};

}
