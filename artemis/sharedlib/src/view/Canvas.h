//
// Created by michi on 8/26/25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/xhui/Panel.h>

class Session;

namespace artemis::view {

class SceneRenderer;


class RenderNode : public xhui::Panel {
public:
	Session* session;

	explicit RenderNode();
};


class Canvas : public RenderNode {
public:
	explicit Canvas();

	void set_child(RenderNode* n);

private:
	RenderNode* child = nullptr;
};

class LayoutOverlay : public RenderNode {
public:
	explicit LayoutOverlay();
	void set_children(const Array<RenderNode*>& c);

private:
	Array<RenderNode*> children;
};

class LayoutNested : public RenderNode {
public:
	explicit LayoutNested();
	void set_children(RenderNode* main, const Array<RenderNode*>& c, int mode);

private:
	RenderNode* _main = nullptr;
	Array<RenderNode*> children;
	Array<xhui::Panel*> dummies;
};

}
