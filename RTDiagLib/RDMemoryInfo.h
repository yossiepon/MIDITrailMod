//******************************************************************************
//
// RTDiagLib / RDMemoryInfo
//
// System memory and process memory collector.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "RDInterfaces.h"
#include <psapi.h>

class RDMemoryInfo : public IRDStartupComponent, public IRDIntervalPollingComponent
{
public:
	RDMemoryInfo() = default;
	virtual ~RDMemoryInfo() = default;

	void CollectStartup() override;
	void CollectIntervalPolling() override;
	DWORD GetPollingIntervalMs() const override { return 2000; }
};
