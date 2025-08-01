/*
 * Picture.h
 *
 *  Created on: 04.01.2020
 *      Author: michi
 */

#pragma once

#include <lib/ygraphics/graphics-fwd.h>
#include "Node.h"
#include "../lib/any/any.h"

namespace gui {

class Picture : public Node {
public:
	Picture(const rect &r, shared<ygfx::Texture> tex, const rect &source, shared<ygfx::Shader> shader);
	Picture(const rect &r, shared<ygfx::Texture> tex, const rect &source = rect::ID);
	~Picture() override;

	void __init__(const rect &r, shared<ygfx::Texture> tex, const rect &source);
	void __delete__() override;

	rect source;

	float bg_blur;
	float angle;

	shared<ygfx::Shader> shader;
	shared<ygfx::Texture> texture;
	Any shader_data;
};

}
