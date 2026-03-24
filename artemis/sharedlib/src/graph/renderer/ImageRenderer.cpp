//
// Created by michi on 02.22.26.
//

#include "ImageRenderer.h"
#include <Session.h>
#include <view/DrawingHelper.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/Context.h>

#include "lib/os/msg.h"

namespace artemis::graph {

ImageRenderer::ImageRenderer(Session* s) : RenderEmitterNode(s, "ImageRenderer") {
	if (!s->ctx)
		msg_error("no ctx");
	material = new yrenderer::Material();
	material->roughness = 1;
	material->metal = 0;
	material->albedo = White;
	material->emission = Black;//White;
	material->pass0.cull_mode = ygfx::CullMode::NONE;
	material->textures.add(new ygfx::Texture);
	vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");
}

ImageRenderer::~ImageRenderer() = default;

void ImageRenderer::on_process() {
	auto image = in_image.value();
	if (!image)
		return;

	material->textures[0]->write(*image);
	vertex_buffer->create_quad({0, (float)image->width, -(float)image->height, 0});

	send_out();
}

void ImageRenderer::on_emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) {
	auto vb = vertex_buffer.get();
	session->drawing_helper->draw_mesh(params, rvd, mat4::rotation_x(pi), vb, material.get());
}

base::optional<Box> ImageRenderer::bounding_box() const {
	auto image = in_image.value();
	if (!image)
		return base::None;

	return Box{{0, 0, 0}, {(float)image->width, (float)image->height, 0}};
}



} // graph