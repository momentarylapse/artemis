
#include <ModeDefault.h>
#include <storage/format/Format.h>
#include "Session.h"
#include "view/ArtemisWindow.h"
#include <lib/kaba/lib/lib.h>
#include "lib/os/msg.h"

string AppVersion = "0.0.1";
string AppName = "Artemis";

void* app = nullptr;

namespace hui {
	string get_language_s(const string& lang) {
		return "";
	}
}

base::future<Session*> emit_empty_session(Session* parent);

int xhui_main(const Array<string>& args) {
	try {
		xhui::init(args, "edward");
	} catch (Exception &e) {
		msg_error(e.message());
		return 1;
	}

	kaba::init();

	auto s = create_session();
	emit_empty_session(s).then([] (Session* ss) {
		ss->set_mode(new ModeDefault(ss));
	});

	try {
		xhui::run();
	} catch (Exception& e) {
		msg_error(e.message());
	}
	return 0;
}

