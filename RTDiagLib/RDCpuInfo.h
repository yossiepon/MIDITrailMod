#pragma once

#include "RDInterfaces.h"

class RDCpuInfo : public IRDStartupComponent
{
public:
	RDCpuInfo() = default;
	virtual ~RDCpuInfo() = default;

	void CollectStartup() override;
};
