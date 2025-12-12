#include <lib/kabaexport/KabaExporter.h>
#include <lib/kaba/Context.h>
#include <plugins/PluginManager.h>

namespace kaba {
	class Context;
	extern Context *_secret_lib_context_;
}

extern "C" {
	__attribute__ ((visibility ("default")))
	void export_symbols(kaba::Exporter* e) {
		// ...
		kaba::default_context = e->ctx;
		kaba::_secret_lib_context_ = e->secret_lib_context;

		artemis::PluginManager::export_kaba(e);
	}
}

