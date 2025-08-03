//
// Created by Michael Ankele on 2025-03-10.
//

#include "VolumeRenderer.h"
#include <Session.h>
#include <data/mesh/GeometrySphere.h>
#include <data/mesh/GeometryCube.h>
#include <view/DrawingHelper.h>
#include <lib/yrenderer/base.h>

namespace artemis::graph {

VolumeRenderer::VolumeRenderer(Session* s) : RendererNode(s, "VolumeRenderer") {
	material = new yrenderer::Material(s->ctx);
	material->pass0.shader_path = "volume.shader";
	material->pass0.mode = yrenderer::TransparencyMode::FUNCTIONS;
	material->pass0.source = ygfx::Alpha::SOURCE_ALPHA;
	material->pass0.destination = ygfx::Alpha::SOURCE_INV_ALPHA;
	material->textures.add(s->ctx->tex_white);

	material_solid = new yrenderer::Material(s->ctx);
	material_solid->pass0.shader_path = "volume-surface.shader";
	material_solid->textures.add(s->ctx->tex_white);
}

void VolumeRenderer::on_process() {
	auto f = in_field.value();
	if (!f)
		return;

	if (!vertex_buffer)
		vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");
	int nx = f->sampling_mode == data::SamplingMode::PerCell ? f->grid.nx : f->grid.nx + 1;
	int ny = f->sampling_mode == data::SamplingMode::PerCell ? f->grid.ny : f->grid.ny + 1;
	int nz = f->sampling_mode == data::SamplingMode::PerCell ? f->grid.nz : f->grid.nz + 1;
	if (!tex)
		tex = new ygfx::VolumeTexture(nx, ny, nz, "r:f32");
	tex->set_options("wrap=clamp,magfilter=nearest");
	if (f->type == data::ScalarType::Float32)
		tex->writex(&f->v32.v[0], nx, ny, nz, "r:f32");
	material->textures[0] = tex.get();
	material_solid->textures[0] = tex.get();

	GeometryCube cube({0,0,0}, vec3::EX, vec3::EY, vec3::EZ, 1, 1, 1);
	cube.build(vertex_buffer.get());

	out_draw(RenderData{active(), f->grid.bounding_box(), nullptr, [this] (const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
		draw_win(params, win, rvd);
	}});
}


void VolumeRenderer::draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win, yrenderer::RenderViewData& rvd) {
	auto f = in_field.value();
	if (!f)
		return;

	if (true) {
		auto _material = material.get();
		if (solid())
			_material = material_solid.get();

		mat4 matrix(f->grid.dx * (float)f->grid.nx, f->grid.dy * (float)f->grid.ny, f->grid.dz * (float)f->grid.nz);
		//session->drawing_helper->draw_mesh(params, rvd, matrix, vertex_buffer.get(), material.get());


		auto shader = rvd.get_shader(_material, 0, "default", "");
		auto& rd = rvd.start(params, matrix, shader, *_material, 0, ygfx::PrimitiveTopology::TRIANGLES, vertex_buffer.get());

		auto cm = color_map();

		if (solid())
			for (auto& c: cm.colors)
				c.a = 1;

#ifdef USING_VULKAN
		params.command_buffer->push_constant(0, sizeof(color)*cm.colors.num, &cm.colors[0]);
		params.command_buffer->push_constant(64, sizeof(float)*cm.values.num, &cm.values[0]);
		params.command_buffer->push_constant(80, 4, &cm.colors.num);
#else
		//shader->set_floats("pattern0", &t->texture_scale[0].x, 3);
		//shader->set_floats("pattern1", &t->texture_scale[1].x, 3);
		shader->set_int("map_count", 0);
#endif
		rd.draw_triangles(params, vertex_buffer.get());
	} else {
		GeometrySphere mesh(v_0, 1, 2);
		mesh.build(vertex_buffer.get());

		if (f->sampling_mode == artemis::data::SamplingMode::PerCell) {
			for (int i=0; i<f->grid.nx; i++)
				for (int j=0; j<f->grid.ny; j++)
					for (int k=0; k<f->grid.nz; k++) {
						float v = f->value32(i, j, k);
						session->drawing_helper->draw_mesh(params, rvd,
							mat4::translation(f->grid.cell_center(i, j, k)) * mat4::scale(v, v, v),
							vertex_buffer.get(),
							session->drawing_helper->material_selection);
					}
		} else if (f->sampling_mode == artemis::data::SamplingMode::PerVertex) {
			for (int i=0; i<=f->grid.nx; i++)
				for (int j=0; j<=f->grid.ny; j++)
					for (int k=0; k<=f->grid.nz; k++) {
						float v = f->value32(i, j, k);
						session->drawing_helper->draw_mesh(params, rvd,
							mat4::translation(f->grid.vertex(i, j, k)) * mat4::scale(v, v, v),
							vertex_buffer.get(),
							session->drawing_helper->material_selection);
					}
		}
	}
}

} // graph