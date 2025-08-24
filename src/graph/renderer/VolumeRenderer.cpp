//
// Created by Michael Ankele on 2025-03-10.
//

#include "VolumeRenderer.h"
#include <Session.h>
#include <data/mesh/GeometrySphere.h>
#include <data/mesh/GeometryCube.h>
#include <view/DrawingHelper.h>
#include <lib/any/conversion.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/helper/Bindable.h>

#include "lib/base/iter.h"

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
	if (!tex or nx != tex->width or ny != tex->height or nz != tex->depth)
		tex = new ygfx::VolumeTexture(nx, ny, nz, "r:f32");
	tex->set_options("wrap=clamp,magfilter=nearest");
#ifdef USING_VULKAN
	if (f->type == data::ScalarType::Float32)
		tex->writex(&f->v32.v[0], nx, ny, nz, "r:f32");
#else
	if (f->type == data::ScalarType::Float32)
		tex->write_float(f->v32.v);
#endif
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

		// "scale" color map
		for (auto& v: cm.values)
			v /= (float)scale();

		if (solid())
			for (auto& c: cm.colors)
				c.a = 1;

		Any data;
		for (const auto& [i, c]: enumerate(cm.colors)) {
			data.dict_set(format("map_colors[%d]:%d", i, i*16), color_to_any(c));
			data.dict_set(format("map_values[%d]:%d", i, 64+i*4), cm.values[i]);
		}
		data.dict_set("map_count:80", cm.colors.num);
		yrenderer::apply_shader_data(params, shader, data);
		rd.draw_triangles(params, vertex_buffer.get());
	} else {
		GeometrySphere mesh(v_0, 1, 2);
		mesh.build(vertex_buffer.get());

		if (f->sampling_mode == artemis::data::SamplingMode::PerCell) {
			for (int i=0; i<f->grid.nx; i++)
				for (int j=0; j<f->grid.ny; j++)
					for (int k=0; k<f->grid.nz; k++) {
						float v = f->_value32(i, j, k);
						session->drawing_helper->draw_mesh(params, rvd,
							mat4::translation(f->grid.cell_center(i, j, k)) * mat4::scale(v, v, v),
							vertex_buffer.get(),
							session->drawing_helper->material_selection);
					}
		} else if (f->sampling_mode == artemis::data::SamplingMode::PerVertex) {
			for (int i=0; i<=f->grid.nx; i++)
				for (int j=0; j<=f->grid.ny; j++)
					for (int k=0; k<=f->grid.nz; k++) {
						float v = f->_value32(i, j, k);
						session->drawing_helper->draw_mesh(params, rvd,
							mat4::translation(f->grid.vertex(i, j, k)) * mat4::scale(v, v, v),
							vertex_buffer.get(),
							session->drawing_helper->material_selection);
					}
		}
	}
}

} // graph