//******************************************************************************
//
// RTDiagLib / RDOsInfo
//
// Operating system information collector.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "RDInterfaces.h"

class RDOsInfo : public IRDStartupComponent
{
public:
	RDOsInfo() = default;
	virtual ~RDOsInfo() = default;

	void CollectStartup() override;
};
