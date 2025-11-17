//
// Created by michi on 11/15/25.
//

#include "MergableAction.h"

MergableAction::MergableAction() {
	time_stamp = std::chrono::high_resolution_clock::now();
}

bool MergableAction::is_recent_enough() const {
	return std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - time_stamp).count() < 2.0;
}




