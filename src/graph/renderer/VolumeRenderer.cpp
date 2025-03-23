//
// Created by Michael Ankele on 2025-03-10.
//

#include "VolumeRenderer.h"
#include <Session.h>
#include <data/mesh/GeometrySphere.h>
#include <view/DrawingHelper.h>

namespace graph {

VolumeRenderer::VolumeRenderer(Session* s) : RendererNode(s, "VolumeRenderer") {}

void VolumeRenderer::process() {
	auto f = in_field.value();
	if (!f)
		return;
	out_draw(RenderData{f->grid.bounding_box()});
}


void VolumeRenderer::draw_win(const RenderParams& params, MultiViewWindow* win) {
	auto f = in_field.value();
	if (!f)
		return;

	if (!vertex_buffer)
		vertex_buffer = new VertexBuffer("3f,3f,2f");

	GeometrySphere mesh(v_0, 1, 2);
	mesh.build(vertex_buffer.get());

	for (int i=0; i<f->grid.nx; i++)
		for (int j=0; j<f->grid.ny; j++)
			for (int k=0; k<f->grid.nz; k++) {
				float v = f->value32(i, j, k);
				session->drawing_helper->draw_mesh(params, win->rvd,
					mat4::translation(f->grid.cell_center(i, j, k)) * mat4::scale(v, v, v),
					vertex_buffer.get(),
					session->drawing_helper->material_selection);
			}
}

} // graph