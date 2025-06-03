/*
 * FormatArtemis.h
 *
 *  Created on: 03.06.2025
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATARTEMIS_H_
#define SRC_STORAGE_FORMAT_FORMATARTEMIS_H_

#include "Format.h"
#include <graph/Graph.h>

class FormatArtemis: public TypedFormat<artemis::graph::DataGraph> {
public:
	explicit FormatArtemis(Session *s);

	void _load(const Path &filename, artemis::graph::DataGraph *data, bool deep) override;
	void _save(const Path &filename, artemis::graph::DataGraph *data) override;
};

#endif /* SRC_STORAGE_FORMAT_FORMATARTEMIS_H_ */
