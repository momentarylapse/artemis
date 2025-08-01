/*----------------------------------------------------------------------------*\
| Nix view                                                                     |
| -> camera etc...                                                             |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#pragma once

namespace hui {
	class Event;
}

class Image;
struct vec2;

namespace nix {

class Texture;
class DepthBuffer;


void _cdecl set_projection_perspective();
void _cdecl set_projection_perspective_ext(const vec2 &center, const vec2 &size_1, float z_min, float z_max);
void _cdecl set_projection_ortho_relative();
void _cdecl set_projection_ortho_pixel();
void _cdecl set_projection_ortho_ext(const vec2 &center, const vec2 &map_size, float z_min, float z_max);
void _cdecl set_projection_matrix(const mat4 &mat);

void _cdecl set_model_matrix(const mat4 &mat);
void _cdecl set_view_matrix(const mat4 &view_mat);

void _cdecl set_viewport(const rect &area);

void _cdecl set_scissor(const rect &r);

void _cdecl screen_shot_to_image(Image &image);


void start_frame_hui(Context *gl, hui::Event* e);
void end_frame_hui();
#if HAS_LIB_GLFW
void start_frame_glfw(Context *gl, void *win);
void end_frame_glfw();
#endif

};

#endif

