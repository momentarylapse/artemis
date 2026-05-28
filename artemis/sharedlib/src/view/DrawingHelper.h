//
// Created by michi on 25.01.25.
//

#ifndef DRAWINGHELPER_H
#define DRAWINGHELPER_H


#include <lib/ygraphics/graphics-fwd.h>
#include <lib/base/optional.h>
#include <lib/image/color.h>
#include <lib/math/vec2.h>
#include <lib/math/vec3.h>
#include <lib/math/mat4.h>
#include <lib/math/rect.h>
#include <lib/xhui/Context.h>

namespace yrenderer {
	class Context;
	struct RenderViewData;
	struct RenderParams;
	class Material;
}
class Painter;

class DrawingHelper {
public:
	explicit DrawingHelper(xhui::Context* ctx1, yrenderer::Context* ctx2);
	void set_color(const color& color);
	color _color;

	void set_line_width(float width);
	float _line_width;

	void clear(const yrenderer::RenderParams& params, const color& c);

	void draw_lines(const Array<vec3>& points, bool contiguous = true);

	static Array<vec2> spline(const vec2& a, const vec2& b, const vec2& c, const vec2& d);

	xhui::Context* xhui_ctx;
	yrenderer::Context* ctx;

	rect target_area;
	mat4 projection, view;

	ygfx::Shader* shader = nullptr;
#ifdef USING_VULKAN
	vulkan::GraphicsPipeline* pipeline = nullptr;
	vulkan::DescriptorSet* dset = nullptr;
#endif
};



#endif //DRAWINGHELPER_H
