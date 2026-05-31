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

#include "lib/yrenderer/MaterialManager.h"


class Storage;
class Progress;
class DrawingHelper;
namespace yrenderer {
	class TextureManager;
	class ShaderManager;
	class LineHelper;
}
namespace xhui {
	class Window;
}

namespace dataflow {
	class Graph;
}

namespace artemis::graph {
	class DataGraph;
}


class Session : public obs::Node<VirtualBase> {
public:
	Session();
	~Session() override;

	void create_initial_resources(yrenderer::Context *ctx);

	void on_command(const string &id);
	void on_close();

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

	xhui::Window* win;

	yrenderer::Context* ctx;
	yrenderer::ShaderManager* shader_manager;
	yrenderer::TextureManager* texture_manager;
	yrenderer::MaterialManager* material_manager;
	owned<yrenderer::LineHelper> line_helper;

	Storage *storage;

	Progress *progress;

	os::Timer timer;
	double t, dt;
	bool simulation_active = false;

	base::promise<Session*> promise_started;

	owned<artemis::graph::DataGraph> data;
	dataflow::Graph* graph;

	double simulation_update_dt = 0.1; // "outside" / how often to perform steps
};

Session *create_session(bool with_window=true);

