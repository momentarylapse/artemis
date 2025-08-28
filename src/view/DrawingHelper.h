//
// Created by michi on 25.01.25.
//

#ifndef DRAWINGHELPER_H
#define DRAWINGHELPER_H


#include <helper/ResourceManager.h>
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

struct TextLayout {
	struct Part {
		string text;
		float font_size;
		base::optional<color> col;
		bool bold;
		bool italic;
		vec2 pos;
		rect box;
	};
	Array<Part> parts;
	rect box() const;

	static TextLayout from_format_string(const string& s, float font_size = -1);
};

class DrawingHelper {
public:
	explicit DrawingHelper(xhui::Context* ctx1, yrenderer::Context* ctx2);
	void set_color(const color& color);
	color _color;

	void set_line_width(float width);
	float _line_width;

	void clear(const yrenderer::RenderParams& params, const color& c);

	void draw_lines(const Array<vec3>& points, bool contiguous = true);
	void draw_circle(const vec3& center, const vec3& axis, float r);

	void draw_mesh(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, const mat4& matrix, ygfx::VertexBuffer* vb, yrenderer::Material* material, int pass_no = 0, const string& vertex_module = "default");

	static void draw_boxed_str(Painter* p, const vec2& pos, const string& str, int align = -1);

	//static void draw_data_points(Painter* p, MultiViewWindow* win, const DynamicArray& a, MultiViewType kind, const base::optional<Hover>& hover);
	static void draw_spline(Painter* p, const vec2& a, const vec2& b, const vec2& c, const vec2& d);

	static Array<vec2> spline(const vec2& a, const vec2& b, const vec2& c, const vec2& d);
	static void draw_text_layout(Painter* p, const vec2& pos, const TextLayout& l);
	static void draw_text_layout_with_box(Painter* p, const vec2& pos, const TextLayout& l, const color& bg, float padding=7, float roundness=7);

	xhui::Context* xhui_ctx;
	yrenderer::Context* ctx;

	rect target_area;
	mat4 projection, view;

	ygfx::Shader* shader = nullptr;
#ifdef USING_VULKAN
	vulkan::GraphicsPipeline* pipeline = nullptr;
	vulkan::DescriptorSet* dset = nullptr;
#endif

	yrenderer::Material* material_hover;
	yrenderer::Material* material_selection;
	yrenderer::Material* material_creation;
};



#endif //DRAWINGHELPER_H
