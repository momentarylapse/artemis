//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

namespace polymesh {
	struct Mesh;
}

namespace artemis::data {
struct ScalarField;
}

namespace artemis::processing {
polymesh::Mesh iso_surface(const data::ScalarField& f, float t0);
}
