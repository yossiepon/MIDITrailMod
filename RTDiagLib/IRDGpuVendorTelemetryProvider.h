//******************************************************************************
//
// RTDiagLib / IRDGpuVendorTelemetryProvider
//
// GPU vendor telemetry provider interface and data structures.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#pragma once

#include <memory>
#include <cstdint>

struct RDGpuVendorTelemetryData
{
	bool isAvailable;

	struct Field {
		bool supported;
		double value;
	};

	Field temperatureCelsius;
	Field coreClockMHz;
	Field memoryClockMHz;
	Field usagePercent;
	Field powerWatts;
	Field fanSpeedRPM;

	Field hotspotTemperatureCelsius;
	Field voltageVolts;
	Field intakeTemperatureCelsius;
	Field vramTemperatureCelsius;
};

class IRDGpuVendorTelemetryProvider
{
public:
	virtual ~IRDGpuVendorTelemetryProvider() = default;
	virtual bool Initialize() = 0;
	virtual void Shutdown() = 0;
	virtual void CollectTelemetry(RDGpuVendorTelemetryData& data) = 0;
};

std::unique_ptr<IRDGpuVendorTelemetryProvider> CreateRDGpuVendorTelemetryProvider(uint32_t vendorId);
