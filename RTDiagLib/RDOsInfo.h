#pragma once

#include "RDInterfaces.h"

class RDOsInfo : public IRDStartupComponent
{
public:
	RDOsInfo() = default;
	virtual ~RDOsInfo() = default;

	void CollectStartup() override;
};
