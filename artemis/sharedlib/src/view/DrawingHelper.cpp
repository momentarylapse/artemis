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


yrenderer::Material* create_material(yrenderer::Context* ctx, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false) {
	auto material = ctx->material_manager->load("");
	material->albedo = albedo;
	material->roughness = roughness;
	material->metal = metal;
	material->emission = emission;
	material->textures = {ctx->tex_white};
	if (transparent) {
		material->pass0.cull_mode = ygfx::CullMode::NONE;
		material->pass0.mode = yrenderer::TransparencyMode::FUNCTIONS;
		material->pass0.source = ygfx::Alpha::SOURCE_ALPHA;
		material->pass0.destination = ygfx::Alpha::SOURCE_INV_ALPHA;
		material->pass0.z_buffer = false;
	}
	return material;
}

DrawingHelper::DrawingHelper(xhui::Context* _ctx1, yrenderer::Context* _ctx2) {
	xhui_ctx = _ctx1;
	ctx = _ctx2;

	ui_scale = xhui_ctx->window->ui_scale;

	/*light = new Light(White, -1, -1);
	light->owner = new Entity;
	light->owner->ang = quaternion::rotation({1,0,0}, 0.5f);
	light->light.harshness = 0.5f;*/

	try {
		material_hover = create_material(ctx, {0.3f, 0,0,0}, 0.9f, 0, White, true);
		material_selection = create_material(ctx, {0.3f, 0,0,0}, 0.9f, 0, Red, true);
		material_creation = create_material(ctx, {0.3f, 0,0.5f,0}, 0.9f, 0, color(1,0,0.5f,0), true);
	} catch(Exception& e) {
		msg_error(e.message());
	}

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

void DrawingHelper::draw_circle(const vec3& center, const vec3& axis, float r) {
	int N = 128;
	Array<vec3> points;
	vec3 e1 = axis.ortho() * r;
	vec3 e2 = vec3::cross(axis, e1);
	for (int i=0; i<=N; i++) {
		float w = (float)i / (float)N * 2 * pi;
		points.add(center + e1 * cos(w) + e2 * sin(w));
	}
	draw_lines(points);
}


void DrawingHelper::clear(const yrenderer::RenderParams& params, const color& c) {
#ifdef USING_VULKAN
	auto cb = params.command_buffer;
	cb->clear(params.area, {c}, 1.0);
#else
	nix::clear(c);
#endif
}

void DrawingHelper::draw_mesh(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, const mat4& matrix, ygfx::VertexBuffer* vertex_buffer, yrenderer::Material* material, int pass_no, const string& vertex_module) {
	auto shader = rvd.get_shader(material, pass_no, vertex_module, "");
	auto& rd = rvd.start(params, matrix, shader, *material, pass_no, ygfx::PrimitiveTopology::TRIANGLES, vertex_buffer);
	rd.draw_triangles(params, vertex_buffer);
}

void DrawingHelper::draw_boxed_str(Painter* p, const vec2& _pos, const string& str, int align) {
	vec2 size = p->get_str_size(str);
	vec2 pos = _pos;
	if (align == 0)
		pos.x -= size.x / 2;
	if (align == 1)
		pos.x -= size.x;
	p->set_color(xhui::Theme::_default.background_button);
	p->set_roundness(7);
	p->draw_rect(rect(pos, pos + size).grow(7));
	p->set_color(xhui::Theme::_default.text_label);
	p->set_roundness(0);
	p->draw_str(pos, str);
}

/*void DrawingHelper::draw_data_points(Painter* p, MultiViewWindow* win, const DynamicArray& _a, MultiViewType kind, const base::optional<Hover>& hover) {
	int _hover = -1;
	if (hover and hover->type == kind)
		_hover = hover->index;
	auto& a = const_cast<DynamicArray&>(_a);
	for (int i=0; i<a.num; i++) {
		const auto v = static_cast<multiview::SingleData*>(a.simple_element(i));
		p->set_color(v->is_selected ? Red : Blue);
		auto p1 = win->project(v->pos);
		float r = 2;
		if (i == _hover)
			r = 4;
		p->draw_rect({p1.x - r,p1.x + r, p1.y - r,p1.y + r});
	}
}*/

void DrawingHelper::draw_spline(Painter* p, const vec2& a, const vec2& b, const vec2& c, const vec2& d) {
	p->draw_lines(spline(a, b, c, d));
}

Array<vec2> DrawingHelper::spline(const vec2& a, const vec2& b, const vec2& c, const vec2& d) {
	Array<vec2> s = {a, b - a, d, d - c};
	Array<vec2> points;
	for (float t=0; t<1; t+=0.05f)
		points.add(cubic_spline(s, t));
	points.add(d);
	return points;
}







