//
// Created by michi on 8/22/25.
//

#pragma once

#include <lib/xhui/Panel.h>

namespace artemis::view {

class NodeListPanel : public xhui::Panel {
public:
	Array<string> classes_filtered;

	explicit NodeListPanel();
};

}