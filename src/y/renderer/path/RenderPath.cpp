//
// Created by michi on 1/3/25.
//

#include "RenderPath.h"
#include <lib/yrenderer/base.h>
#include "../world/WorldRenderer.h"
#include <lib/yrenderer/scene/pass/ShadowRenderer.h>
#include <lib/yrenderer/post/MultisampleResolver.h>
#include <lib/yrenderer/post/HDRResolver.h>
#include "../world/WorldRendererForward.h"
#include "../world/WorldRendererDeferred.h"
#ifdef USING_VULKAN
	#include "../world/WorldRendererVulkanRayTracing.h"
#endif
#include "../helper/LightMeter.h"
#include <lib/yrenderer/target/TextureRenderer.h>
#include <lib/yrenderer/helper/CubeMapSource.h>
#include "../../helper/ResourceManager.h"
#include <lib/yrenderer/ShaderManager.h>
#include <lib/ygraphics/graphics-impl.h>
#include <world/Camera.h>
#include <world/Model.h>
#include <world/Terrain.h>
#include <world/World.h>
#include <y/EngineData.h>
#include <y/Entity.h>
#include <y/ComponentManager.h>
#include <Config.h>
#include <lib/profiler/Profiler.h>
#include <lib/math/Box.h>
#include <lib/os/msg.h>
#include <lib/os/time.h>
#include <renderer/world/emitter/WorldModelsEmitter.h>
#include <renderer/world/emitter/WorldTerrainsEmitter.h>
#include <renderer/world/emitter/WorldUserMeshesEmitter.h>
#include <renderer/world/emitter/WorldInstancedEmitter.h>
#include <renderer/world/emitter/WorldSkyboxEmitter.h>
#include <world/components/MultiInstance.h>
#include <lib/yrenderer/scene/MeshEmitter.h>
#include "world/Light.h"

using namespace yrenderer;


HDRResolver* create_hdr_resolver(Context* ctx, Camera *cam, ygfx::Texture* tex, ygfx::DepthBuffer* depth) {
	return new HDRResolver(ctx, cam, tex, depth);
}

WorldRenderer* create_world_renderer(Context* ctx, SceneView& scene_view, RenderPathType type) {
#ifdef USING_VULKAN
	if (type == RenderPathType::PathTracing)
		return new WorldRendererVulkanRayTracing(ctx, scene_view, engine.width, engine.height);
#endif
	if (type == RenderPathType::Deferred)
		return new WorldRendererDeferred(ctx, scene_view, engine.width, engine.height);
	return new WorldRendererForward(ctx, scene_view);
}

float global_shadow_box_size;

RenderPath::RenderPath(Context* ctx, RenderPathType _type, Camera* _cam) : Renderer(ctx, "path") {
	type = _type;
	cam = _cam;
	shadow_box_size = config.get_float("shadow.boxsize", 2000);
	shadow_resolution = config.get_int("shadow.resolution", 1024);
	global_shadow_box_size = shadow_box_size;

	scene_view.cam = cam;

	resource_manager->shader_manager->default_shader = "default.shader";
	resource_manager->shader_manager->load_shader_module("module-basic-interface.shader");
	resource_manager->shader_manager->load_shader_module("module-basic-data.shader");
	const string light_sources = config.get_str("renderer.light_sources", "default");
	resource_manager->shader_manager->load_shader_module(format("module-light-sources-%s.shader", light_sources));
	const string shadows_method = config.get_str("shadow.quality", "pcf-hardening");
	resource_manager->shader_manager->load_shader_module(format("module-shadows-%s.shader", shadows_method));
	const string lighting_method = config.get_str("renderer.lighting", "pbr");
	resource_manager->shader_manager->load_shader_module(format("module-lighting-%s.shader", lighting_method));
	resource_manager->shader_manager->load_shader_module("module-vertex-default.shader");
	resource_manager->shader_manager->load_shader_module("module-vertex-animated.shader");
	resource_manager->shader_manager->load_shader_module("module-vertex-instanced.shader");
	resource_manager->shader_manager->load_shader_module("module-vertex-lines.shader");
	resource_manager->shader_manager->load_shader_module("module-vertex-points.shader");
	resource_manager->shader_manager->load_shader_module("module-vertex-fx.shader");
	resource_manager->shader_manager->load_shader_module("module-geometry-lines.shader");
	resource_manager->shader_manager->load_shader_module("module-geometry-points.shader");


	if (type != RenderPathType::PathTracing) {
		// not sure this is a good idea...
		auto e = new Entity;
		cube_map_source = new CubeMapSource;
		cube_map_source->owner = e;
		cube_map_source->cube_map = new ygfx::CubeMap(cube_map_source->resolution, "rgba:i8");

		scene_view.cube_map = cube_map_source->cube_map;
	}


	world_renderer = create_world_renderer(ctx, scene_view, type);

	if (type != RenderPathType::PathTracing)
		create_shadow_renderer();

	create_cube_renderer();

	if (type != RenderPathType::Direct)
		create_post_processing(world_renderer);
}

RenderPath::~RenderPath() = default;

void RenderPath::prepare_basics() {
	check_terrains(cam_main->owner->pos);
}

void RenderPath::check_terrains(const vec3& cam_pos) {
	auto& terrains = ComponentManager::get_list_family<Terrain>();
	if (terrains.num == 0)
		return;

	if (updater.num == 0) {
		auto u = new XTerrainVBUpdater;
		u->terrain = terrains[0];
		u->vb = new ygfx::VertexBuffer("3f,3f,2f");;
		updater.add(u);

		// first time: complete update!
		terrains[0]->prepare_draw(cam_pos);
		return;
	}

	os::Timer timer;
	for (auto u: updater) {
		while (timer.peek() < 0.0003f) {
			int r = u->iterate(cam_pos);
			if (r == 0)
				break;
			if (r == 2) {
				auto vb = u->vb;
				u->vb = terrains[0]->vertex_buffer.give();
				terrains[0]->vertex_buffer = vb;
				break;
			}
		}
	}

	/*for (auto *t: terrains) {
		t->prepare_draw(cam_pos);
	}*/

}

void RenderPath::create_shadow_renderer() {
	shadow_renderer = new ShadowRenderer(ctx, &scene_view, {
		new WorldModelsEmitter(ctx),
		new WorldTerrainsEmitter(ctx),
		new WorldUserMeshesEmitter(ctx),
		new WorldInstancedEmitter(ctx)});
	scene_view.shadow_maps.add(shadow_renderer->cascades[0].depth_buffer);
	scene_view.shadow_maps.add(shadow_renderer->cascades[1].depth_buffer);
	add_sub_task(shadow_renderer.get());
}

void RenderPath::create_cube_renderer() {
	cube_map_renderer = new CubeMapRenderer(ctx, scene_view, {
		new WorldSkyboxEmitter(ctx),
		new WorldModelsEmitter(ctx),
		new WorldTerrainsEmitter(ctx),
		new WorldUserMeshesEmitter(ctx),
		new WorldInstancedEmitter(ctx)});
}


void RenderPath::create_post_processing(Renderer* source) {

	auto hdr_tex = new ygfx::Texture(engine.width, engine.height, "rgba:f16");
	hdr_tex->set_options("wrap=clamp,minfilter=nearest");
	hdr_tex->set_options("magfilter=" + config.resolution_scale_filter);
	auto hdr_depth = new ygfx::DepthBuffer(engine.width, engine.height, "d:f32");

	hdr_resolver = create_hdr_resolver(ctx, cam, hdr_tex, hdr_depth);

#ifdef USING_VULKAN
	config.antialiasing_method = AntialiasingMethod::NONE;
#endif

	if (config.antialiasing_method == AntialiasingMethod::MSAA) {
		msg_error("yes msaa");

		msg_write("ms tex:");
		auto tex_ms = new ygfx::TextureMultiSample(engine.width, engine.height, 4, "rgba:f16");
		msg_write("ms depth:");
		auto depth_ms = new ygfx::TextureMultiSample(engine.width, engine.height, 4, "d:f32");
		msg_write("ms renderer:");
		//auto depth_ms = new nix::RenderBuffer(engine.width, engine.height, 4, "ds:u24i88");
		texture_renderer = new TextureRenderer(ctx, "world-tex", {tex_ms, depth_ms}, {"samples=4"});

		multisample_resolver = new MultisampleResolver(ctx, tex_ms, depth_ms, hdr_tex, hdr_depth);
	} else {
		msg_error("no msaa");
		texture_renderer = new TextureRenderer(ctx, "world-tex", {hdr_tex, hdr_depth});
	}

	texture_renderer->add_child(source);

	light_meter = new LightMeter(ctx, hdr_tex);
}



void RenderPath::render_into_cubemap(CubeMapSource& source) {
	cube_map_renderer->set_source(&source);
	//cube_map_renderer->render(RenderParams::WHATEVER);
}


void RenderPath::suggest_cube_map_pos() {
	if (!cube_map_source)
		return;
	if (world.ego) {
		cube_map_source->owner->pos = world.ego->pos;
		cube_map_source->min_depth = 200;
		if (auto m = world.ego->get_component<Model>())
			cube_map_source->min_depth = m->prop.radius * 1.1f;
		return;
	}
	auto& list = ComponentManager::get_list_family<Model>();
	float max_score = 0;
	cube_map_source->owner->pos = scene_view.cam->view_matrix() * vec3(0,0,1000);
	cube_map_source->min_depth = 1000;
	for (auto m: list)
		for (auto mat: m->material) {
			float score = mat->metal;
			if (score > max_score) {
				max_score = score;
				cube_map_source->owner->pos = m->owner->pos;
				cube_map_source->min_depth = m->prop.radius;
			}
		}
}

void RenderPath::render_cubemaps(const RenderParams &params) {
	suggest_cube_map_pos();
	auto cube_map_sources = ComponentManager::get_list<CubeMapSource>();
	cube_map_sources.add(cube_map_source);
	for (auto& source: cube_map_sources) {
		if (source->update_rate <= 0)
			continue;
		source->counter ++;
		if (source->counter >= source->update_rate) {
			//render_into_cubemap(*source);
			cube_map_renderer->set_source(source);
			cube_map_renderer->render(params);
			source->counter = 0;
		}
	}
}

// keep this outside the drawing function, making sure it only gets called once per frame!
void RenderPath::prepare_instanced_matrices() {
	//profiler::begin(ch_pre);
	auto& list = ComponentManager::get_list_family<MultiInstance>();
	for (auto *mi: list) {
		if (!mi->ubo_matrices)
			mi->ubo_matrices = new ygfx::UniformBuffer(MAX_INSTANCES * sizeof(mat4));
		//mi->ubo_matrices->update_array(mi->matrices);
		mi->ubo_matrices->update_part(&mi->matrices[0], 0, min(mi->matrices.num, MAX_INSTANCES) * sizeof(mat4));
	}
	//profiler::end(ch_pre);
}

void RenderPath::prepare(const RenderParams& params) {
	prepare_basics();
	prepare_instanced_matrices();
	scene_view.choose_lights();
	scene_view.choose_shadows();

	if (type != RenderPathType::PathTracing)
		world_renderer->prepare(params);
	// FIXME replace by ...
	//scene_view.cam->update_matrix_cache(params.desired_aspect_ratio);

	if (cube_map_source)
		scene_view.cube_map = cube_map_source->cube_map;

	if (shadow_renderer)
		shadow_renderer->render(params);


	//cam->update_matrix_cache(params.desired_aspect_ratio);

	if (type != RenderPathType::PathTracing)
		render_cubemaps(params);

	if (texture_renderer) {
		texture_renderer->set_area(dynamicly_scaled_area(texture_renderer->frame_buffer.get()));
		texture_renderer->render(params);
	}

	if (multisample_resolver)
		multisample_resolver->render(params);

	if (hdr_resolver)
		hdr_resolver->prepare(params);


	if (light_meter and hdr_resolver) {
		light_meter->active = hdr_resolver->cam and hdr_resolver->cam->auto_exposure;
		if (light_meter->active) {
			light_meter->read();
			light_meter->setup();
			light_meter->adjust_camera(hdr_resolver->cam);
		}
	}
}

void RenderPath::draw(const RenderParams& params) {
	if (hdr_resolver)
		hdr_resolver->draw(params);
	else
		world_renderer->draw(params);
}

RenderPath* create_render_path(Context* ctx, Camera *cam) {
	string type = config.get_str("renderer.path", "forward");

	if (type == "direct")
		return new RenderPath(ctx, RenderPathType::Direct, cam);
	if (type == "deferred")
		return new RenderPath(ctx, RenderPathType::Deferred, cam);
	if (type == "pathtracing" or type == "raytracing")
		return new RenderPath(ctx, RenderPathType::PathTracing, cam);
	return new RenderPath(ctx, RenderPathType::Forward, cam);
}

