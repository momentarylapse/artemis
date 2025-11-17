/*
 * ActionManager.h
 *
 *  Created on: 05.03.2012
 *      Author: michi
 */

#pragma once

#include "Action.h"
#include "../data/Data.h"
#include <lib/pattern/Observable.h>
#include <lib/base/pointer.h>

class Data;
class Action;
class ActionGroup;

class ActionManager : public obs::Node<VirtualBase> {
	friend class Action;
public:
	explicit ActionManager(Data *_data);
	~ActionManager() override;

	obs::source out_failed{this, "failed"};
	obs::source out_saved{this, "saved"};

	void reset();
	void enable(bool _enabled);

	void* execute(xfer<Action> a);
	void undo();
	void redo();

	void begin_group(const string& name);
	void end_group();

	bool preview(Action* a);
	void clear_preview();

	bool undoable();
	bool redoable();
	bool is_save();
	void mark_current_as_save();

	string error_message;
	string error_location;

private:
	void add(Action* a);
	Data* data;
	owned_array<Action> history;
	int cur_pos;
	int save_pos;

	int cur_level;
	bool enabled;

	// group
	int cur_group_level;
	ActionGroup* cur_group;

	// preview
	Action* _preview;
};
