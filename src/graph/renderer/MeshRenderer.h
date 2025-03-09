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

class Session;

namespace graph {

class MeshRenderer : public Node {
public:
	explicit MeshRenderer(Session* s);
	~MeshRenderer() override;

	void process() override;

	InPort<PolygonMesh> in_mesh{this, "mesh"};

	Setting<float> roughness{this, "roughness", 0.5f};
	Setting<float> metal{this, "metal", 0.5f};
	Setting<color> albedo{this, "albedo", White};
	Setting<color> emission{this, "emission", Black};

	owned<VertexBuffer> vertex_buffer;
	owned<Material> material;
};

} // graph

#endif //MESHRENDERER_H
