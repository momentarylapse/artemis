#if HAS_LIB_VULKAN

#include "Context.h"
#include "../os/msg.h"
#include "../image/image.h"
#include "graphics-impl.h"

namespace ygfx {

Context::Context(vulkan::Instance* _instance, vulkan::Device* _device) {
	instance = _instance;
	device = _device;
}

Context::~Context() {
	delete device;
	delete instance;
}


}

#endif
