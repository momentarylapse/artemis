#pragma once

#include <lib/base/base.h>
#include "graphics-fwd.h"

namespace ygfx {

class Context {
public:
#if HAS_LIB_VULKAN
	explicit Context(vulkan::Instance* instance, vulkan::Device* device);
#else
	explicit Context(nix::Context* ctx);
#endif
	~Context();

#if HAS_LIB_VULKAN
	vulkan::Instance* instance = nullptr;
	vulkan::Device* device = nullptr;
#else
	nix::Context* ctx = nullptr;
#endif

	Texture* tex_white;
	Texture* tex_black;
	void _create_default_textures();
};

}

