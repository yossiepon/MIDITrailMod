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
