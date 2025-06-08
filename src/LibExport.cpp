
#include <ModeDefault.h>
#include <lib/dataflow/Node.h>
#include <graph/NodeFactory.h>
#include <graph/Graph.h>
#include <storage/format/Format.h>
#include "Session.h"
#include <view/ArtemisWindow.h>
#include <plugins/PluginManager.h>
#include <lib/xhui/Application.h>
#include <lib/kaba/kaba.h>
#include <lib/os/msg.h>
#include <y/plugins/PluginManager.h>

#include "storage/Storage.h"

string AppVersion = "0.0.1";
string AppName = "Artemis";

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


class KabaExporter {
public:
	virtual ~KabaExporter() = default;
	virtual void declare_class_size(const string& name, int size) = 0;
	virtual void declare_enum(const string& name, int value) = 0;
	virtual void declare_class_element(const string& name, int offset) = 0;
	virtual void link(const string& name, void* p) = 0;
	virtual void link_virtual(const string& name, void* p, void* instance) = 0;
};


Session* export_create_session() {
	return create_session();
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

int xhui_main(const Array<string>& args) {
	return 0;
}

extern "C" {
	__attribute__ ((visibility ("default")))
	void export_symbols(KabaExporter* e) {
		e->link("create_session", (void*)&create_session);
		e->link("start", (void*)&export_start);
		e->link("run", (void*)&export_run);
	}
}

