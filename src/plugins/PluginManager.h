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

	static base::map<const std::type_info*, const kaba::Class*> type_map;
};

template<class T>
const kaba::Class* get_class() {
	return PluginManager::type_map[&typeid(T)];
}

}

#endif //PLUGINMANAGER_H
