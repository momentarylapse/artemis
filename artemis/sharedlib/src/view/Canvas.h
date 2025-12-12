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

	explicit RenderNode(Session* session);
};


class Canvas : public RenderNode {
public:
	explicit Canvas(Session* s);

	void set_child(RenderNode* n);

	Session* session;

private:
	RenderNode* child = nullptr;
};

class LayoutGrid : public RenderNode {
public:
	explicit LayoutGrid(Session* s);
	void set_children(const Array<RenderNode*>& c, bool horizontal);
	void configure(int spacing, int margin);

private:
	bool horizontal = false;
	Array<RenderNode*> children;
};

class LayoutOverlay : public RenderNode {
public:
	explicit LayoutOverlay(Session* s);
	void set_children(const Array<RenderNode*>& c);

private:
	Array<RenderNode*> children;
};

class LayoutNested : public RenderNode {
public:
	explicit LayoutNested(Session* s);
	void set_children(RenderNode* main, const Array<RenderNode*>& c, int mode);

private:
	RenderNode* _main = nullptr;
	Array<RenderNode*> children;
	Array<xhui::Panel*> dummies;
};

}
