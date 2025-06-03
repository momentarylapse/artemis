//
// Created by Michael Ankele on 2025-03-10.
//

#include "VolumeRenderer.h"
#include <Session.h>
#include <data/mesh/GeometrySphere.h>
#include <data/mesh/GeometryCube.h>
#include <view/DrawingHelper.h>
#include <y/renderer/base.h>

namespace artemis::graph {

VolumeRenderer::VolumeRenderer(Session* s) : RendererNode(s, "VolumeRenderer") {
	material = new Material(s->resource_manager);
	material->pass0.shader_path = "volume.shader";
	material->textures.add(tex_white);
}

void VolumeRenderer::process() {
	auto f = in_field.value();
	if (!f)
		return;

	if (!vertex_buffer)
		vertex_buffer = new VertexBuffer("3f,3f,2f");
	int nx = f->sampling_mode == data::SamplingMode::PerCell ? f->grid.nx : f->grid.nx + 1;
	int ny = f->sampling_mode == data::SamplingMode::PerCell ? f->grid.ny : f->grid.ny + 1;
	int nz = f->sampling_mode == data::SamplingMode::PerCell ? f->grid.nz : f->grid.nz + 1;
	if (!tex)
		tex = new VolumeTexture(nx, ny, nz, "r:f32");
	tex->set_options("wrap=clamp,magfilter=nearest");
	if (f->type == data::ScalarType::Float32)
		tex->writex(&f->v32.v[0], nx, ny, nz, "r:f32");
	material->textures[0] = tex.get();

	GeometryCube cube({0,0,0}, vec3::EX, vec3::EY, vec3::EZ, 1, 1, 1);
	cube.build(vertex_buffer.get());

	out_draw(RenderData{f->grid.bounding_box()});
}


void VolumeRenderer::draw_win(const RenderParams& params, MultiViewWindow* win, RenderViewData& rvd) {
	auto f = in_field.value();
	if (!f)
		return;

	if (true) {
		mat4 m(f->grid.dx * (float)f->grid.nx, f->grid.dy * (float)f->grid.ny, f->grid.dz * (float)f->grid.nz);
		session->drawing_helper->draw_mesh(params, rvd, m, vertex_buffer.get(), material.get());
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