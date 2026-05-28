//
// Created by michi on 25.01.25.
//

#include "DrawingHelper.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/font.h>
#include <lib/ygraphics/Context.h>
#include <lib/yrenderer/MaterialManager.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/scene/RenderViewData.h>
#include <lib/math/mat4.h>
#include <lib/math/vec2.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <lib/math/interpolation.h>
#include <cmath>


static float ui_scale = 1.0f;

DrawingHelper::DrawingHelper(xhui::Context* _ctx1, yrenderer::Context* _ctx2) {
	xhui_ctx = _ctx1;
	ctx = _ctx2;

	ui_scale = xhui_ctx->window->ui_scale;

#ifdef USING_VULKAN
	shader = vulkan::Shader::create(
		R"foodelim(
<Layout>
	version = 430
	bindings = [[sampler]]
	pushsize = 96
</Layout>
<VertexShader>

//#extension GL_ARB_separate_shader_objects : enable

layout(push_constant, std430) uniform Parameters {
	mat4 matrix;
	vec4 color;
	vec2 size;
	float radius;
	float softness;
} params;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 out_pos; // camera space
layout(location = 1) out vec2 out_uv;

void main() {
	gl_Position = params.matrix * vec4(in_position, 1);
	out_pos = gl_Position;
	out_uv = in_uv;
}
</VertexShader>
<FragmentShader>

layout(push_constant, std140) uniform Parameters {
	mat4 matrix;
	vec4 color;
	vec2 size;
	float radius;
	float softness;
} params;

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D tex0;

void main() {
	out_color = texture(tex0, in_uv);
	out_color *= params.color;
}
</FragmentShader>
)foodelim");
//#ifdef USING_VULKAN
	dset = ctx->pool->create_set(shader);
	dset->set_texture(0, ctx->tex_white);
	dset->update();

	pipeline = new vulkan::GraphicsPipeline(shader, xhui_ctx->render_pass, 0, ygfx::PrimitiveTopology::TRIANGLES, xhui_ctx->aux->vb);
	//pipeline->set_z(false, false);
	pipeline->set_culling(ygfx::CullMode::NONE);
	pipeline->rebuild();
#endif
}


void DrawingHelper::set_color(const color& color) {
	_color = color;
}

void DrawingHelper::set_line_width(float width) {
	_line_width = width;
}



static void add_vb_line(Array<ygfx::Vertex1>& vertices, const vec3& a, const vec3& b, const rect& area, float line_width) {
	float w = area.width();
	float h = area.height();
	vec2 ba_pixel = vec2((b.x - a.x) * w, (b.y - a.y) * h);
	vec2 dir_pixel = ba_pixel.normalized();
	vec2 o_pixel = dir_pixel.ortho();
	vec3 r = vec3(o_pixel.x/ w, o_pixel.y/ h, 0) * (line_width / 2) * ui_scale;
	vec3 a0 = a - r;
	vec3 a1 = a + r;
	vec3 b0 = b - r;
	vec3 b1 = b + r;
	vertices.add({a0, v_0, 0,0});
	vertices.add({a1, v_0, 0,0});
	vertices.add({b0, v_0, 0,0});
	vertices.add({b0, v_0, 0,0});
	vertices.add({a1, v_0, 0,0});
	vertices.add({b1, v_0, 0,0});
}

void DrawingHelper::draw_lines(const Array<vec3>& points, bool contiguous) {
#ifdef USING_VULKAN
	auto vb = xhui_ctx->aux->get_line_vb();
	Array<ygfx::Vertex1> vertices;
	mat4 m = projection * view;
	if (contiguous) {
		for (int i=0; i<points.num-1; i++)
			add_vb_line(vertices, m.project(points[i]), m.project(points[i+1]), target_area, _line_width);
	} else {
		for (int i=0; i<points.num-1; i+=2)
			add_vb_line(vertices, m.project(points[i]), m.project(points[i+1]), target_area, _line_width);
	}
	vb->update(vertices);

	struct Parameters {
		mat4 matrix;
		color col;
		vec2 size;
		float radius, softness;
	};

	Parameters params;
	params.matrix = mat4::ID;
	params.col = _color;
	params.size = {1000,1000};//(float)width, (float)height};
	params.radius = 0;//line_width;
	params.softness = 0;//softness;

	auto cb = xhui_ctx->current_command_buffer();
	cb->bind_pipeline(pipeline);
	cb->push_constant(0, sizeof(params), &params);
	cb->bind_descriptor_set(0, dset);
	cb->draw(vb);
#endif
}


void DrawingHelper::clear(const yrenderer::RenderParams& params, const color& c) {
#ifdef USING_VULKAN
	auto cb = params.command_buffer;
	cb->clear(params.area, {c}, 1.0);
#else
	nix::clear(c);
#endif
}

Array<vec2> DrawingHelper::spline(const vec2& a, const vec2& b, const vec2& c, const vec2& d) {
	Array<vec2> s = {a, b - a, d, d - c};
	Array<vec2> points;
	for (float t=0; t<1; t+=0.05f)
		points.add(cubic_spline(s, t));
	points.add(d);
	return points;
}







