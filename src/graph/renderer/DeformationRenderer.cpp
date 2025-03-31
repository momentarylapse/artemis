#include "DeformationRenderer.h"
#include <Session.h>
#include <data/mesh/GeometrySphere.h>
#include <view/DrawingHelper.h>

namespace artemis::graph {

DeformationRenderer::DeformationRenderer(Session* s) : RendererNode(s, "DeformationRenderer") {}

void DeformationRenderer::process() {
	auto d = in_diff.value();
	if (!d)
		return;

	out_draw(RenderData{d->grid.bounding_box()});
}


void DeformationRenderer::draw_win(const RenderParams& params, MultiViewWindow* win) {

	auto d = in_diff.value();
	if (!d)
		return;

	if (!vertex_buffer)
		vertex_buffer = new VertexBuffer("3f,3f,2f");

	GeometrySphere mesh(v_0, 1, 2);
	mesh.build(vertex_buffer.get());

	float s = scale();
	float r = radius();

	if (d->sampling_mode == artemis::data::SamplingMode::PerCell) {
		for (int i=0; i<d->grid.nx; i++)
			for (int j=0; j<d->grid.ny; j++)
				for (int k=0; k<d->grid.nz; k++) {
					vec3 pos = d->grid.cell_center(i, j, k) + d->value32(i, j, k) * s;
					session->drawing_helper->draw_mesh(params, win->rvd,
						mat4::translation(pos) * mat4::scale(r, r, r),
						vertex_buffer.get(),
						session->drawing_helper->material_selection);
				}
	} else if (d->sampling_mode == artemis::data::SamplingMode::PerVertex) {
		for (int i=0; i<=d->grid.nx; i++)
			for (int j=0; j<=d->grid.ny; j++)
				for (int k=0; k<=d->grid.nz; k++) {
					vec3 pos = d->grid.vertex(i, j, k) + d->value32(i, j, k) * s;
					session->drawing_helper->draw_mesh(params, win->rvd,
						mat4::translation(pos) * mat4::scale(r, r, r),
						vertex_buffer.get(),
						session->drawing_helper->material_selection);
				}
	}
}


} // graph
