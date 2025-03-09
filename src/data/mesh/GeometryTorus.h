/*
 * GeometryTorus.h
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#ifndef GEOMETRYTORUS_H_
#define GEOMETRYTORUS_H_

#include "PolygonMesh.h"
class vec3;

class GeometryTorus : public PolygonMesh
{
public:
	GeometryTorus(const vec3 &pos, const vec3 &axis, float radius1, float radius2, int num_x, int num_y);
	void _cdecl __init__(const vec3 &pos, const vec3 &axis, float radius1, float radius2, int num_x, int num_y);
};

#endif /* GEOMETRYTORUS_H_ */
