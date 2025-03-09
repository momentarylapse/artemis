//
// Created by michi on 09.03.25.
//

#include "MeshRenderer.h"

#include <Session.h>
#include <y/graphics-impl.h>

namespace graph {

	MeshRenderer::MeshRenderer(Session* s) : Node("MeshRenderer") {
		material = new Material(s->resource_manager);
	}

	MeshRenderer::~MeshRenderer() = default;

	void MeshRenderer::process() {
		auto mesh = in_mesh.value();
		if (!mesh)
			return;

		if (!vertex_buffer)
			vertex_buffer = new VertexBuffer("3f,3f,2f");
		mesh->build(vertex_buffer.get());

		material->roughness = roughness();
		material->metal = metal();
		material->albedo = albedo();
		material->emission = emission();

		dirty = false;
	}

} // graph