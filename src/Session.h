/*
 * Session.h
 *
 *  Created on: 12 Sept 2023
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>
#include <lib/base/map.h>
#include <lib/base/future.h>
#include <lib/os/time.h>
#include <lib/os/path.h>
#include <lib/pattern/Observable.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/yrenderer/regions/RegionRenderer.h>


class Mode;
class Storage;
class Progress;
class ResourceManager;
class DrawingHelper;
class ArtemisWindow;



// multiview data
enum class MultiViewType {
	MODEL_SURFACE,
	MODEL_VERTEX,
	MODEL_SKIN_VERTEX,
	MODEL_POLYGON,
	MODEL_EDGE,
	MODEL_BALL,
	MODEL_CYLINDER,
	MODEL_POLYHEDRON,
	SKELETON_BONE,
	WORLD_ENTITY,
	WORLD_OBJECT,
	WORLD_TERRAIN,
	WORLD_TERRAIN_VERTEX,
	WORLD_LIGHT,
	WORLD_LINK,
	WORLD_SCRIPT,
	WORLD_CAMERA,
	WORLD_CAM_POINT,
	WORLD_CAM_POINT_VEL,
	ACTION_MANAGER
};

namespace artemis::graph {
	class Graph;
	class DataGraph;
}


class Session : public obs::Node<VirtualBase> {
public:
	Session();
	~Session() override;

	void create_initial_resources(yrenderer::Context *ctx);

	void set_mode(Mode *m);
	void set_mode_now(Mode *m);

	void on_command(const string &id);
	void on_close();

	Mode *get_mode(int type);

	struct Message {
		enum class Type {
			Info,
			Warning,
			Error
		};
		Type type;
		string text;
	};
	void add_message(Message::Type type, const string &message);
	void remove_message();
	void info(const string &message);
	void warning(const string &message);
	void error(const string &message);
	Array<Message> messages;

	base::future<void> allow_termination();

	ArtemisWindow *win;

	Mode *mode_none;
	Mode *cur_mode = nullptr;
	Array<Mode*> mode_queue;
	Mode *find_mode_base(const string &name);
	template<class M>
	M *find_mode(const string &name) {
		return static_cast<M*>(find_mode_base(name));
	}

	yrenderer::Context *ctx;
	ResourceManager *resource_manager;
	DrawingHelper *drawing_helper;

	Storage *storage;

	Progress *progress;

	base::map<ygfx::Texture*, string> icon_image;
	string get_tex_image(ygfx::Texture *tex);

	os::Timer timer;

	base::promise<Session*> promise_started;

	owned<artemis::graph::DataGraph> data;
	artemis::graph::Graph* graph;

	float simulation_update_dt = 0.1f; // "outside" / how often to perform steps
};

Session *create_session();

