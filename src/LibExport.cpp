#include "KabaExporter.h"
#include <plugins/PluginManager.h>

extern "C" {
	__attribute__ ((visibility ("default")))
	void export_symbols(kaba::Exporter* e) {
		artemis::PluginManager::export_kaba(e);
	}
}

