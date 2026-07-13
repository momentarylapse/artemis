#include <lib/kapi/KabaExporter.h>
#include <lib/kaba/kaba.h>
#include <plugins/PluginManager.h>
#include <lib/os/msg.h>
#include <lib/profiler/Profiler.h>

bool app_init();
bool app_init_core();

KABA_PACKAGE_EXPORT_BEGIN
KABA_PACKAGE_EXPORT void export_symbols(kaba::IExporter* e) {
	kaba::make_context_public(e);

	profiler::init_external(e->context()->get_global_symbol("profiler", "state"));

	artemis::PluginManager::export_kaba(e);

	app_init_core();
//	if (!app_init())
//		msg_error("failed to initialize...");
}
KABA_PACKAGE_EXPORT_END

