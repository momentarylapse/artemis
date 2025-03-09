//
// Created by michi on 09.03.25.
//

#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include "../Node.h"
#include "../Setting.h"
#include "../Port.h"
#include <data/mesh/PolygonMesh.h>
#include <y/graphics-fwd.h>

namespace graph {

class MeshRenderer : public Node {
public:
	MeshRenderer();
	~MeshRenderer() override;

	void process() override;

	InPort<PolygonMesh> in_mesh{this, "mesh"};

	owned<VertexBuffer> vertex_buffer;
};

} // graph

#endif //MESHRENDERER_H
