
#include <ModeDefault.h>
#include <lib/dataflow/Node.h>
#include <graph/NodeFactory.h>
#include <graph/Graph.h>
#include "Session.h"
#include <view/ArtemisWindow.h>
#include <plugins/PluginManager.h>
#include <lib/kaba/kaba.h>
#include <lib/os/msg.h>
#include <y/plugins/PluginManager.h>

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

void start_session_empty(Session* parent) {
	emit_empty_session(parent).then([] (Session* s) {
		if (true) {
			auto canvas = artemis::graph::create_node(s, "Canvas");
			canvas->pos = {300, 600};
			s->graph->add_node(canvas);
		} else {
			auto teapot = artemis::graph::create_node(s, "TeapotMesh");
			teapot->pos = {300, 200};
			s->graph->add_node(teapot);
			auto renderer = artemis::graph::create_node(s, "MeshRenderer");
			renderer->pos = {300, 400};
			//renderer->set("emission", color_to_any(Green));
			s->graph->add_node(renderer);
			auto canvas = artemis::graph::create_node(s, "Canvas");
			canvas->pos = {300, 600};
			s->graph->add_node(canvas);
			s->graph->connect({teapot, 0, renderer, 0});
			s->graph->connect({renderer, 0, canvas, 0});
		}
		s->set_mode(new ModeDefault(s));
	});
}

void start_session_load_file(Session* parent, const Path& filename) {
	emit_empty_session(parent).then([filename] (Session* s) {
		s->set_mode(new ModeDefault(s));

		string ext = filename.extension();
		if (ext == "artemis") {
			s->storage->load(filename, s->data.get());
		} else if (ext == "kaba") {
			auto m = kaba::default_context->load_module(filename.absolute());
			typedef void (*f_p)();
			if (auto f = (f_p)m->match_function("main", "void", {})) {
				f();
			} else {
				s->set_message(format("script %s does not contain 'func main()'", filename));
			}
		} else {
			s->set_message(format("unknown file extension: %s", filename));
		}
	});
}

Session* export_start() {
	try {
		xhui::init({}, "artemis");
	} catch (Exception &e) {
		msg_error(e.message());
		return nullptr;
	}

	kaba::init();
	try {
		artemis::PluginManager::init();
	} catch (Exception &e) {
		msg_error(e.message());
	}

	auto s = create_session();
	artemis::graph::init_factory();
	start_session_empty(s);
	return s;
}
void export_run() {
	try {
		xhui::run();
	} catch (Exception& e) {
		msg_error(e.message());
	}
}

namespace os::app {
	int main(const Array<string>& args) {
		try {
			xhui::init(args, "artemis");
		} catch (Exception &e) {
			msg_error(e.message());
			return 1;
		}

		kaba::init();
		try {
			artemis::PluginManager::init();
		} catch (Exception &e) {
			msg_error(e.message());
		}

		auto s = create_session();
		artemis::graph::init_factory();
		if (args.num >= 2) {
			start_session_load_file(s, args[1]);
		} else {
			start_session_empty(s);
		}

		try {
			xhui::run();
		} catch (Exception& e) {
			msg_error(e.message());
		}
		return 0;
	}
}

