#include <graph/NodeFactory.h>
#include "Session.h"
#include <plugins/PluginManager.h>
#include <lib/os/msg.h>

string AppVersion = "0.1.0";
string AppName = "Artemis";

base::future<Session*> emit_empty_session(Session* parent);

void app_init_core() {
	try {
		artemis::PluginManager::init();
	} catch (Exception &e) {
		msg_error(e.message());
	}
	artemis::graph::init_factory();
}


