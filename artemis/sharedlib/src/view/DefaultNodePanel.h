//
// Created by Michael Ankele on 2025-03-12.
//

#pragma once

#include <lib/xhui/Panel.h>

class Session;
namespace dataflow {
class Node;
}

class DefaultNodePanel : public xhui::Panel {
public:
	explicit DefaultNodePanel(Session* s, dataflow::Node* n);
	dataflow::Node* node;
	Session* session;
};

