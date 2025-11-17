//
// Created by michi on 11/15/25.
//

#pragma once

#include "Action.h"
#include <lib/os/time.h>


class MergableAction : public Action {
public:
	MergableAction();

	// merge this (later) into previous
	// AFTER both have been executed!
	virtual bool try_merge_into(Action* previous) = 0;

	bool is_recent_enough() const;

	std::chrono::high_resolution_clock::time_point time_stamp;
};

