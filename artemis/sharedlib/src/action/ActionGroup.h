/*
 * ActionGroup.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#pragma once

#include "Action.h"

class Data;
class ActionManager;

class ActionGroup: public Action {
	friend class ActionManager;
public:
	ActionGroup();
	~ActionGroup() override;

	virtual void* compose(Data* d) {
		return nullptr;
	}

	void *execute(Data* d) override;
	void undo(Data* d) override;
	void redo(Data* d) override;

	void abort(Data* d) override;
	bool was_trivial() const override;

protected:
	void* add_sub_action(Action* a, Data* d);

private:
	Array<Action*> action;
};

class ActionGroupManual : public ActionGroup {
	friend class ActionManager;
public:
	explicit ActionGroupManual(const string &name) {
		_name_ = name;
	}

	virtual string name() const {
		return _name_;
	}
private:
	string _name_;
};
