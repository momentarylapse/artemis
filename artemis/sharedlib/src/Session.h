/*
 * Session.h
 *
 *  Created on: 12 Sept 2023
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>
#include <lib/base/future.h>
#include <lib/os/time.h>
#include <lib/os/path.h>
#include <lib/pattern/Observable.h>


class Progress;
class DrawingHelper;
namespace yrenderer {
	class Context;
	class TextureManager;
	class ShaderManager;
	class MaterialManager;
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

	struct Message {
		enum class Type {
			Info,
			Warning,
			Error
		};
		Type type;
		string text;
	};
	Array<Message> messages;

	xhui::Window* win = nullptr;

	yrenderer::Context* ctx = nullptr;
	yrenderer::ShaderManager* shader_manager = nullptr;
	yrenderer::TextureManager* texture_manager = nullptr;
	yrenderer::MaterialManager* material_manager = nullptr;
	yrenderer::LineHelper* line_helper = nullptr;

	Progress* progress = nullptr;

	os::Timer timer;
	double t, dt;
	bool simulation_active = false;

	base::promise<Session*> promise_started;

	owned<artemis::graph::DataGraph> data;
	dataflow::Graph* graph;

	double simulation_update_dt = 0.1; // "outside" / how often to perform steps
};

Session* create_session();

