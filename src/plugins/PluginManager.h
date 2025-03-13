//
// Created by michi on 09.03.25.
//

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <typeinfo>
#include <lib/base/base.h>
#include <lib/base/map.h>

class Path;
namespace kaba {
	class Class;
}

namespace artemis {

class PluginManager {
public:
	static void init();
	static void export_kaba();
	static void import_kaba();
	static void find_plugins();

	static Path directory();

	static base::map<const std::type_info*, const kaba::Class*> type_map;
	static base::map<string, Path> plugin_classes;
	static void* create_instance(const string& name);
};

template<class T>
const kaba::Class* get_class() {
	return PluginManager::type_map[&typeid(T)];
}

}

#endif //PLUGINMANAGER_H
