//******************************************************************************
//
// RTDiagLib / RDWmiInfo
//
// WMI-based hardware information collector (GPU driver version, machine type).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "RDInterfaces.h"

class RDWmiInfo : public IRDStartupComponent
{
public:
	RDWmiInfo() = default;
	virtual ~RDWmiInfo() = default;

	void CollectStartup() override;
};
