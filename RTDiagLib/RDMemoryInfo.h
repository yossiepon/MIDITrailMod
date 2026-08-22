#pragma once

#include "RDInterfaces.h"

class RDMemoryInfo : public IRDStartupComponent
{
public:
	RDMemoryInfo() = default;
	virtual ~RDMemoryInfo() = default;

	void CollectStartup() override;
};
