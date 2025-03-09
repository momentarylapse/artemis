/*
 * Artemis.h
 *
 *  Created on: 09.03.2025
 *      Author: michi
 */

#ifndef ARTEMIS_H_
#define ARTEMIS_H_

#include "lib/xhui/xhui.h"

class PluginManager;


class ArtemisApp : public xhui::Application {
public:
	ArtemisApp();
	//hui::AppStatus on_startup(const Array<string> &arg) override;


	PluginManager *plugins;
};

extern ArtemisApp *app;

#endif /* ARTEMIS_H_ */
