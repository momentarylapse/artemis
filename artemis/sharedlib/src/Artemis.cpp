
#include <lib/dataflow/Node.h>
#include <graph/NodeFactory.h>
#include <graph/Graph.h>
#include "Session.h"
#include <view/ArtemisWindow.h>
#include <plugins/PluginManager.h>
#include <lib/kaba/kaba.h>
#include <lib/os/msg.h>

#include "storage/Storage.h"

string AppVersion = "0.0.1";
string AppName = "Artemis";

void* app = nullptr;

namespace hui {
	string get_language_s(const string& lang) {
		return "";
	}
}

base::future<Session*> emit_empty_session(Session* parent);
void add_default_graph(Session* s) {
	if (false) {
		auto canvas = artemis::graph::create_node(s, "Canvas");
		canvas->pos = {300, 600};
		s->graph->add_node(canvas);
	} else {
		auto teapot = artemis::graph::create_node(s, "TeapotMesh");
		teapot->pos = {300, 200};
		s->graph->add_node(teapot);
		auto renderer = artemis::graph::create_node(s, "MeshRenderer");
		renderer->pos = {300, 350};
		//renderer->set("emission", color_to_any(Green));
		s->graph->add_node(renderer);
		auto scene = artemis::graph::create_node(s, "SceneRenderer");
		scene->pos = {300, 500};
		s->graph->add_node(scene);
		auto canvas = artemis::graph::create_node(s, "Canvas");
		canvas->pos = {300, 650};
		s->graph->add_node(canvas);
		s->graph->connect(teapot, 0, renderer, 0);
		s->graph->connect(renderer, 0, scene, 0);
		s->graph->connect(scene, 0, canvas, 0);
	}
}

void session_load_file(Session* s, const Path& filename) {
	string ext = filename.extension();
	if (ext == "artemis") {
		s->storage->load(filename, s->data.get());
	} else if (ext == "kaba") {
		auto m = kaba::default_context->load_module(filename.absolute(), false);
		typedef void (*f_p)();
		if (auto f = (f_p)m->match_function("main", "void", {})) {
			f();
		} else {
			s->error(format("script %s does not contain 'func main()'", filename));
		}
	} else {
		s->error(format("unknown file extension: %s", filename));
	}
}

void app_init_core() {
	try {
		artemis::PluginManager::init();
	} catch (Exception &e) {
		msg_error(e.message());
	}
	artemis::graph::init_factory();
}

namespace os::app {
	int main(const Array<string>&) {
		return 0;
	}
}

