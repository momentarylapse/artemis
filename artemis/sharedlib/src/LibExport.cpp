#include <lib/kapi/KabaExporter.h>
#include <lib/kaba/kaba.h>
#include <plugins/PluginManager.h>
#include <lib/os/msg.h>

bool app_init();
bool app_init_core();

extern "C" {
	__attribute__ ((visibility ("default")))
	void export_symbols(kaba::IExporter* e) {
		kaba::make_context_public(e);

		artemis::PluginManager::export_kaba(e);

		app_init_core();
	//	if (!app_init())
	//		msg_error("failed to initialize...");
	}
}

