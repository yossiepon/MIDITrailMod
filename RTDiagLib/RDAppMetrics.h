#pragma once

#include "RDInterfaces.h"

class RDAppMetrics : public IRDFrameComponent
{
public:
	RDAppMetrics() = default;
	virtual ~RDAppMetrics() = default;

	void CollectFrame() override;
};
