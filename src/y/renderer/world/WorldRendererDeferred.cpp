/*
 * WorldRenderer.cpp
 *
 *  Created on: 07.08.2020
 *      Author: michi
 */

#include "WorldRendererDeferred.h"
#include <lib/yrenderer/target/TextureRenderer.h>
#include <lib/yrenderer/scene/pass/ShadowRenderer.h>
#include <lib/yrenderer/post/ThroughShaderRenderer.h>
#include <lib/yrenderer/base.h>
#include "../path/RenderPath.h"
#include <lib/os/msg.h>
#include <lib/math/random.h>
#include <lib/math/vec4.h>
#include <lib/math/vec2.h>
#include <lib/profiler/Profiler.h>
#include <renderer/world/emitter/WorldInstancedEmitter.h>
#include <renderer/world/emitter/WorldModelsEmitter.h>
#include <renderer/world/emitter/WorldSkyboxEmitter.h>
#include <renderer/world/emitter/WorldTerrainsEmitter.h>
#include <renderer/world/emitter/WorldUserMeshesEmitter.h>
#include <renderer/world/emitter/WorldParticlesEmitter.h>
#include <world/World.h>
#include "../../helper/ResourceManager.h"
#include "../../world/Camera.h"
#include <lib/yrenderer/ShaderManager.h>
#include "../../Config.h"
#include <lib/ygraphics/graphics-impl.h>


using namespace yrenderer;
using namespace ygfx;

WorldRendererDeferred::WorldRendererDeferred(yrenderer::Context* ctx, SceneView& scene_view, int width, int height) : WorldRenderer(ctx, "world/def", scene_view) {

	auto tex1 = new Texture(width, height, "rgba:f16"); // diffuse
	auto tex2 = new Texture(width, height, "rgba:f16"); // emission
	auto tex3 = new Texture(width, height, "rgba:f16"); // pos
	auto tex4 = new Texture(width, height, "rgba:f16"); // normal,reflectivity
	auto depth = new DepthBuffer(width, height, "ds:u24i8");
	gbuffer_textures = {tex1, tex2, tex3, tex4, depth};
	for (auto a: weak(gbuffer_textures))
		a->set_options("wrap=clamp,magfilter=nearest,minfilter=nearest");


	gbuffer_renderer = new TextureRenderer(ctx, "gbuf", gbuffer_textures);
	gbuffer_renderer->clear_z = true;
	gbuffer_renderer->clear_colors = {color(-1, 0,1,0)};


	resource_manager->shader_manager->load_shader_module("forward/module-surface.shader");
	resource_manager->shader_manager->load_shader_module("deferred/module-surface.shader");

	auto shader_gbuffer_out = resource_manager->shader_manager->load_shader("deferred/out.shader");
//	if (!shader_gbuffer_out->link_uniform_block("SSAO", 13))
//		msg_error("SSAO");

	out_renderer = new ThroughShaderRenderer(ctx, "out", shader_gbuffer_out);
	out_renderer->bind_textures(0, {tex1, tex2, tex3, tex4, depth});


	Array<vec4> ssao_samples;
	Random r;
	for (int i=0; i<64; i++) {
		auto v = r.dir() * pow(r.uniform01(), 1);
		ssao_samples.add(vec4(v.x, v.y, abs(v.z), 0));
	}
	ssao_sample_buffer = new UniformBuffer(ssao_samples.num * sizeof(vec4));
	ssao_sample_buffer->update_array(ssao_samples);

	ch_gbuf_out = profiler::create_channel("gbuf-out", channel);
	ch_trans = profiler::create_channel("trans", channel);

	scene_renderer_background = new SceneRenderer(ctx, RenderPathType::Forward, scene_view);
	scene_renderer_background->add_emitter(new WorldSkyboxEmitter(ctx));
	add_child(scene_renderer_background.get());

	scene_renderer = new SceneRenderer(ctx, RenderPathType::Deferred, scene_view);
	scene_renderer->add_emitter(new WorldModelsEmitter(ctx));
	scene_renderer->add_emitter(new WorldTerrainsEmitter(ctx));
	scene_renderer->add_emitter(new WorldUserMeshesEmitter(ctx));
	scene_renderer->add_emitter(new WorldInstancedEmitter(ctx));
	scene_renderer->allow_transparent = false;
	gbuffer_renderer->add_child(scene_renderer.get());

	scene_renderer_trans = new SceneRenderer(ctx, RenderPathType::Forward, scene_view);
	scene_renderer_trans->add_emitter(new WorldModelsEmitter(ctx));
	scene_renderer_trans->add_emitter(new WorldUserMeshesEmitter(ctx));
	scene_renderer_trans->add_emitter(new WorldParticlesEmitter(ctx));
	scene_renderer_trans->allow_opaque = false;
	add_child(scene_renderer_trans.get());
}

void WorldRendererDeferred::prepare(const yrenderer::RenderParams& params) {
	profiler::begin(ch_prepare);

	auto sub_params = params.with_target(gbuffer_renderer->frame_buffer.get());

	gbuffer_renderer->set_area(dynamicly_scaled_area(gbuffer_renderer->frame_buffer.get()));

	scene_renderer_background->set_view_from_camera(params, scene_view.cam);
	scene_renderer_background->prepare(params); // keep drawing into direct target

	scene_renderer->set_view_from_camera(sub_params, scene_view.cam);
	scene_renderer->prepare(sub_params);

	scene_renderer_trans->set_view_from_camera(params, scene_view.cam);
	scene_renderer_trans->prepare(params); // keep drawing into direct target


	gbuffer_renderer->render(params);

	profiler::end(ch_prepare);
}

void WorldRendererDeferred::draw(const RenderParams& params) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

	scene_renderer_background->draw(params);

	render_out_from_gbuffer(gbuffer_renderer->frame_buffer.get(), params);

	// transparency
#ifdef USING_OPENGL
	msg_todo("deferred rendering in OpenGL is broken");
	auto& rvd = scene_renderer_trans->rvd;

	profiler::begin(ch_trans);
	bool flip_y = params.target_is_window;
	mat4 m = flip_y ? mat4::scale(1,-1,1) : mat4::ID;
	auto cam = scene_view.cam;
	//cam->update_matrices(params.desired_aspect_ratio);
	nix::set_projection_matrix(m * cam->projection_matrix(params.desired_aspect_ratio)); // TODO
	nix::bind_uniform_buffer(BINDING_LIGHT, rvd.ubo_light.get());
	nix::set_view_matrix(cam->view_matrix()); // TODO
	nix::set_z(true, true);
	nix::set_front(flip_y ? nix::Orientation::CW : nix::Orientation::CCW);

	scene_renderer_trans->draw(params);
	nix::set_cull(nix::CullMode::BACK);
	nix::set_front(nix::Orientation::CW);

	nix::set_z(false, false);
	//nix::set_projection_matrix(mat4::ID);
	//nix::set_view_matrix(mat4::ID);
	profiler::end(ch_trans);
#endif

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

void WorldRendererDeferred::render_out_from_gbuffer(FrameBuffer *source, const RenderParams& params) {
	profiler::begin(ch_gbuf_out);

	auto& data = out_renderer->bindings.shader_data;

#ifdef USING_OPENGL
	if constexpr (SceneRenderer::using_view_space)
		data.dict_set("eye_pos", vec3_to_any(vec3::ZERO));
	else
		data.dict_set("eye_pos", vec3_to_any(scene_view.cam->owner->pos)); // NAH
#endif
	data.dict_set("ambient_occlusion_radius:8", config.ambient_occlusion_radius);
	out_renderer->bind_uniform_buffer(13, ssao_sample_buffer);

	auto& rvd = scene_renderer->rvd;
	out_renderer->bind_uniform_buffer(BINDING_LIGHT, rvd.ubo_light.get());
	for (int i=0; i<gbuffer_textures.num; i++)
		out_renderer->bind_texture(i, gbuffer_textures[i].get());
	out_renderer->bind_texture(BINDING_SHADOW0, scene_view.shadow_maps[0]);
	out_renderer->bind_texture(BINDING_SHADOW1, scene_view.shadow_maps[1]);

	float resolution_scale_x = 1.0f;
	data.dict_set("resolution_scale:0", vec2_to_any(vec2(resolution_scale_x, resolution_scale_x)));

	out_renderer->set_source(dynamicly_scaled_source());
	out_renderer->draw(params);

	// ...
	//geo_renderer->draw_transparent();

	profiler::end(ch_gbuf_out);
}


