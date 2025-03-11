//
// Created by Michael Ankele on 2025-03-11.
//

#pragma once

struct PolygonMesh;

namespace artemis::data {
struct ScalarField;
}

namespace artemis::processing {
PolygonMesh iso_surface(const data::ScalarField& f, float t0);
}
