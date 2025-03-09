//
// Created by michi on 09.03.25.
//

#include "MeshRenderer.h"
#include <y/graphics-impl.h>

namespace graph {

	MeshRenderer::MeshRenderer() : Node("MeshRenderer") {}

	MeshRenderer::~MeshRenderer() = default;

	void MeshRenderer::process() {
		auto mesh = in_mesh.value();
		if (!mesh)
			return;


		if (vertex_buffer)
			return;
		vertex_buffer = new VertexBuffer("3f,3f,2f");
		mesh->build(vertex_buffer.get());
	}

} // graph