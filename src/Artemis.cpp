
#include <ModeDefault.h>
#include <graph/NodeFactory.h>
#include <storage/format/Format.h>
#include "Session.h"
#include <view/ArtemisWindow.h>
#include <plugins/PluginManager.h>
#include <lib/kaba/kaba.h>
#include <lib/os/msg.h>
#include <y/plugins/PluginManager.h>

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
		s->set_mode(new ModeDefault(s));
	});
}

void start_session_load_file(Session* parent, const Path& filename) {
	emit_empty_session(parent).then([filename] (Session* s) {
		s->set_mode(new ModeDefault(s));

		string ext = filename.extension();
		if (ext == "kaba") {
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

int xhui_main(const Array<string>& args) {
	try {
		xhui::init(args, "edward");
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
	graph::init_factory();
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

