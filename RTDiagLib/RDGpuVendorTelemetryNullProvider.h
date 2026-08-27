//******************************************************************************
//
// RTDiagLib / RDGpuVendorTelemetryNullProvider
//
// Null GPU vendor telemetry provider (fallback).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include "IRDGpuVendorTelemetryProvider.h"

class RDGpuVendorTelemetryNullProvider : public IRDGpuVendorTelemetryProvider
{
public:
	bool Initialize() override { return true; }
	void Shutdown() override {}

	void CollectTelemetry(RDGpuVendorTelemetryData& data) override
	{
		data.isAvailable = false;
		data.temperatureCelsius = { false, 0.0 };
		data.coreClockMHz       = { false, 0.0 };
		data.memoryClockMHz     = { false, 0.0 };
		data.usagePercent       = { false, 0.0 };
		data.powerWatts         = { false, 0.0 };
		data.fanSpeedRPM        = { false, 0.0 };
	}
};
