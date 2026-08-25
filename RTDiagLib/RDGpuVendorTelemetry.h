#pragma once

#include "RDInterfaces.h"
#include "IRDGpuVendorTelemetryProvider.h"
#include <memory>

class RDGpuVendorTelemetry : public IRDIntervalPollingComponent
{
public:
	RDGpuVendorTelemetry();
	virtual ~RDGpuVendorTelemetry();

	void SetVendorId(uint32_t vendorId);
	bool InitializeProvider();

	void CollectIntervalPolling() override;
	DWORD GetPollingIntervalMs() const override { return 1000; }

private:
	uint32_t m_vendorId;
	std::unique_ptr<IRDGpuVendorTelemetryProvider> m_provider;
};
