#if HAS_LIB_GL

#include "Context.h"
#include "Window.h"
#include "Painter.h"
#include "../os/msg.h"
#include "../image/image.h"
#include <lib/nix/nix.h>
#include <lib/ygraphics/Context.h>

namespace nix {
	extern bool allow_separate_vertex_arrays;
	mat4 create_pixel_projection_matrix();
}

namespace xhui {

Context::Context(Window* w, ygfx::Context* ctx) {
	window = w;
	context = ctx;
}

Painter* Context::prepare_draw() {
	glfwMakeContextCurrent(window->window);
	int width, height;
	glfwGetWindowSize(window->window, &width, &height);
	int ww, hh;
	glfwGetFramebufferSize(window->window, &ww, &hh);

	const rect area = {0, (float)width, 0, (float)height};
	const rect native_area = {0, (float)ww, 0, (float)hh};
	return new Painter(window, native_area, area);
}

void Context::begin_draw(Painter *p) {
	// in case the event_id::JustBeforeDraw triggers off-screen rendering...
	nix::bind_frame_buffer(context->ctx->default_framebuffer);

	nix::start_frame_glfw(context->ctx, window->window);
	nix::set_projection_matrix(nix::create_pixel_projection_matrix() * mat4::translation({0,0,0.5f}) * mat4::scale(window->ui_scale, window->ui_scale, 1));
	//nix::clear(color(1, 0.15f, 0.15f, 0.3f));
	nix::set_cull(nix::CullMode::NONE);
	nix::set_z(false, false);
}

void Context::end_draw(Painter *p) {
	nix::end_frame_glfw();
	iterate_text_caches();
}



Context* Context::create(Window* window) {
	glfwMakeContextCurrent(window->window);
	nix::allow_separate_vertex_arrays = true;
	nix::default_shader_bindings = false;
	auto ctx = new Context(window, new ygfx::Context(nix::init()));

	ctx->tex_text = new ygfx::Texture();

	ctx->context->_create_default_textures();
	ctx->tex_white = ctx->context->tex_white;
	ctx->tex_black = ctx->context->tex_black;

	ctx->vb = new ygfx::VertexBuffer("3f,3f,2f");
	ctx->vb->create_quad(rect::ID, rect::ID);



#if 0
	<VertexShader>
	#version 330 core
	#extension GL_ARB_separate_shader_objects : enable

	struct Matrix { mat4 model, view, project; };
	/*layout(binding = 0)*/ uniform Matrix matrix;

	layout(location = 0) in vec3 in_position;
	layout(location = 1) in vec3 in_normal;
	layout(location = 2) in vec2 in_uv;

	layout(location = 0) out vec2 out_uv;

	void main() {
		gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
		out_uv = in_uv;
	}

	</VertexShader>
#endif

	ctx->shader = ctx->context->ctx->create_shader(
			R"foodelim(
<Layout>
	version = 420
</Layout>
<VertexShader>

#extension GL_ARB_separate_shader_objects : enable

struct Matrix { mat4 model, view, project; };
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec4 out_pos; // camera space

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_normal = (matrix.view * matrix.model * vec4(in_normal, 0)).xyz;
	out_uv = in_uv;
}
</VertexShader>
<FragmentShader>
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_pos;
layout(binding=0) uniform sampler2D tex0;
uniform vec4 _color_;
out vec4 out_color;

void main() {
	out_color = texture(tex0, in_uv);
	out_color *= _color_;
}
</FragmentShader>
)foodelim");
	ctx->shader->filename = "-my-shader-";


	ctx->shader_round  = ctx->context->ctx->create_shader(
			R"foodelim(
<Layout>
	version = 420
</Layout>
<VertexShader>

#extension GL_ARB_separate_shader_objects : enable

struct Matrix { mat4 model, view, project; };
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec4 out_pos; // camera space

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_normal = (matrix.view * matrix.model * vec4(in_normal, 0)).xyz;
	out_uv = in_uv;
}
</VertexShader>
<FragmentShader>
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_pos;
uniform sampler2D tex0;
uniform vec4 _color_;
uniform vec2 size;
uniform float radius = 0;
uniform float softness = 0;
out vec4 out_color;

void main() {
	out_color = texture(tex0, in_uv);
	out_color *= _color_;
	if (softness >= 0.5) {
		vec2 pp = (abs(in_uv - 0.5) * size - (0.5*size-softness-radius));
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - radius) / softness, 0, 1);
	} else {
		vec2 pp = (abs(in_uv - 0.5) * size - (0.5*size-radius));
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - radius), 0, 1);
	}
	//out_color = vec4(length(pp)/radius, 0, 1, 1);
}
</FragmentShader>
)foodelim");
	ctx->shader_round->filename = "-my-shader-round-";

	ctx->tex_xxx = new nix::Texture();


	return ctx;
}


void Context::rebuild_default_stuff() {
}

void Context::resize(int w, int h) {
	rebuild_default_stuff();
}

}

#endif
