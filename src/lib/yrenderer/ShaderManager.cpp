//
// Created by michi on 7/30/25.
//

#include "ShaderManager.h"
#include <lib/os/filesystem.h>
#include <lib/os/file.h>
#include <lib/os/msg.h>
#include <lib/os/app.h>
#include <lib/image/image.h>
#include <lib/ygraphics/graphics-impl.h>

#ifdef USING_VULKAN
namespace vulkan {
	extern string overwrite_bindings;
	extern int overwrite_push_size;
}
#endif

namespace yrenderer {

Path guess_absolute_path(const Path &filename, const Array<Path> dirs) {
	if (filename.is_empty())
		return Path::EMPTY;

	if (filename.is_absolute())
		return filename;

	for (auto &d: dirs)
		if (os::fs::exists(d | filename))
			return d | filename;

	return Path::EMPTY;
	/*if (engine.ignore_missing_files) {
		msg_error("missing shader: " + filename.str());
		return Shader::load("");
	}
	throw Exception("missing shader: " + filename.str());
	return filename;*/
}


ShaderManager::ShaderManager(ygfx::Context *_ctx, const Path &_shader_dir) {
	ctx = _ctx;
	shader_dir = _shader_dir;
}


xfer<ygfx::Shader> ShaderManager::__load_shader(const Path& path, const string &overwrite_bindings, int overwrite_push_size) {
#ifdef USING_VULKAN
	//msg_write("loading shader: " + str(path));
	vulkan::overwrite_bindings = overwrite_bindings;
	vulkan::overwrite_push_size = overwrite_push_size;
	return ygfx::Shader::load(path);
#else
	return ctx->load_shader(path);
#endif
}

xfer<ygfx::Shader> ShaderManager::__create_shader(const string& source, const string &overwrite_bindings, int overwrite_push_size) {
#ifdef USING_VULKAN
	vulkan::overwrite_bindings = overwrite_bindings;
	vulkan::overwrite_push_size = overwrite_push_size;
	return ygfx::Shader::create(source);
#else
	return ctx->create_shader(source);
#endif
}

shared<ygfx::Shader> ShaderManager::load_shader(const Path& filename) {
	//if (!filename)
	//	TODO default shader?
	//	return __load_shader("");

	Path fn = guess_absolute_path(filename, {shader_dir, os::app::directory_static | "shader"});
	if (!fn) {
		if (ignore_missing_files) {
			msg_error("missing shader: " + str(filename));
			return __load_shader("", "", -1);
		}
		throw Exception("missing shader: " + str(filename));
		//fn = shader_dir | filename;
	}

	for (auto&& [key, s]: shader_map)
		if (key == fn) {
#ifdef USING_VULKAN
			return s;
#else
			return (s->program >= 0) ? s : nullptr;
#endif
		}

	auto s = __load_shader(fn, "", -1);
	if (!s)
		return nullptr;

	shaders.add(s);
	shader_map.add({fn, s});
	return s;
}

string ShaderManager::expand_vertex_shader_source(const string &source, const string &variant) {
	if (source.find("<VertexShader>") >= 0)
		return source;
	//msg_write("INJECTING " + variant);
	return source + format("\n<VertexShader>\n#import vertex-%s\n</VertexShader>", variant);
}

string ShaderManager::expand_fragment_shader_source(const string &source, const string &render_path) {
	if (render_path.num > 0)
		return source.replace("#import surface", "#import surface-" + render_path);
	return source;
}

string ShaderManager::expand_geometry_shader_source(const string &source, const string &variant) {
	if (source.find("<GeometryShader>") >= 0)
		return source;
	//msg_write("INJECTING " + variant);
	return source + format("\n<GeometryShader>\n#import geometry-%s\n</GeometryShader>", variant);
}

shared<ygfx::Shader> ShaderManager::load_surface_shader(const Path& _filename, const string &render_path, const string &vertex_module, const string &geometry_module) {
	//msg_write("load_surface_shader: " + str(_filename) + "  " + render_path + "  " + vertex_module + "  " + geometry_module);
	//select_default_vertex_module("vertex-" + variant);
	//return load_shader(filename);
	auto filename = _filename;
	if (!filename)
		filename = default_shader;



	if (!filename)
		return __load_shader("", "", -1);

	Path fn = guess_absolute_path(filename, {shader_dir, os::app::directory_static | "shader"});
	if (fn.is_empty()) {
		if (ignore_missing_files) {
			msg_error("missing shader: " + str(filename));
			return __load_shader("", "", -1);
		}
		throw Exception("missing shader: " + str(filename));
		//fn = shader_dir | filename;
	}

	Path fnx = fn.with(":" + render_path +  ":" + vertex_module + ":" + geometry_module);
	for (auto&& [key, s]: shader_map)
		if (key == fnx) {
#ifdef USING_VULKAN
			return s;
#else
			return (s->program >= 0) ? s : nullptr;
#endif
		}


	msg_write("loading shader: " + str(fnx));

	string source = expand_vertex_shader_source(os::fs::read_text(fn), vertex_module);
	if (geometry_module != "")
		source = expand_geometry_shader_source(source, geometry_module);
	source = expand_fragment_shader_source(source, render_path);

	auto shader = __create_shader(source, "[[sampler,sampler,sampler,sampler,sampler,sampler,sampler,sampler,ubo,ubo,ubo,ubo,ubo]]", 96);

	//auto s = Shader::load(fn);

	shaders.add(shader);
	shader_map.add({fnx, shader});
	return shader;
}

ygfx::Shader* ShaderManager::create_shader(const string &source) {
	return __create_shader(source, "", -1);
}

void ShaderManager::load_shader_module(const Path& path) {
	Path fn = guess_absolute_path(path, {shader_dir, os::app::directory_static | "shader"});
	if (fn) {
		if (shader_modules.find(fn) >= 0)
			return;
		shader_modules.add(fn);
	}
	load_shader(path);
}



void ShaderManager::clear() {
	shaders.clear();
	shader_map.clear();
}

}
