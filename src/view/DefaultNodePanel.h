//
// Created by Michael Ankele on 2025-03-12.
//

#ifndef DEFAULTNODEPANEL_H
#define DEFAULTNODEPANEL_H

#include <lib/xhui/Panel.h>

namespace dataflow {
class Node;
}

class DefaultNodePanel : public xhui::Panel {
public:
	explicit DefaultNodePanel(dataflow::Node* n);
	dataflow::Node* node;
};


#endif //DEFAULTNODEPANEL_H
