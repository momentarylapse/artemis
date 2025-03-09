#include "HeadlessRendererVulkan.h"

#ifdef USING_VULKAN
#include <helper/PerformanceMonitor.h>
#include <renderer/base.h>
#include "../../graphics-impl.h"


HeadlessRenderer::HeadlessRenderer(vulkan::Device* d, const shared_array<Texture>& tex) : RenderTask("headless")
{
	device = d;
	command_buffer = new CommandBuffer(device->command_pool);
	fence = new vulkan::Fence(device);

	texture_renderer = new TextureRenderer("tex", tex);
}

HeadlessRenderer::~HeadlessRenderer() = default;


void HeadlessRenderer::prepare(const RenderParams &params) {
	Renderer::prepare(params);
	render(params);
}

RenderParams HeadlessRenderer::create_params(const rect& area) const {
	auto p = RenderParams::into_texture(texture_renderer->frame_buffer.get(), area.width() / area.height());
	p.area = area;
	p.command_buffer = command_buffer;
	return p;
}

void HeadlessRenderer::render(const RenderParams& params) {
	const auto p = create_params(texture_renderer->frame_buffer->area());
	command_buffer->begin();
	draw(p);
	device->graphics_queue.submit(command_buffer, {}, {}, fence);
	fence->wait();
	//device->wait_idle();
}

#endif
