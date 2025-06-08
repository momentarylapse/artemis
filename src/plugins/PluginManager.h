//
// Created by michi on 09.03.25.
//

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <lib/base/base.h>
#include <lib/base/map.h>

class Path;
namespace kaba {
	class Class;
	class Exporter;
}

namespace artemis {

class PluginManager {
public:
	static void init();
	static void export_kaba(kaba::Exporter* e);
	static void import_kaba();
	static void find_plugins();

	static Path directory();

	static base::map<string, Path> plugin_classes;
	static void* create_instance(const string& name);
};

}

#endif //PLUGINMANAGER_H
