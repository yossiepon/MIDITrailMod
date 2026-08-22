#pragma once

#include "RDInterfaces.h"

class RDWmiInfo : public IRDStartupComponent
{
public:
	RDWmiInfo() = default;
	virtual ~RDWmiInfo() = default;

	void CollectStartup() override;
};
