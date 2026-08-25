#pragma once

#include "IRDGpuVendorTelemetryProvider.h"
#include <Windows.h>

namespace adlx { class IADLXSystem; }
namespace adlx { class IADLXPerformanceMonitoringServices; }
namespace adlx { class IADLXGPU; }
namespace adlx { class IADLXGPUList; }

class RDGpuVendorTelemetryAdlxProvider : public IRDGpuVendorTelemetryProvider
{
public:
	RDGpuVendorTelemetryAdlxProvider();
	~RDGpuVendorTelemetryAdlxProvider() override;

	bool Initialize() override;
	void Shutdown() override;
	void CollectTelemetry(RDGpuVendorTelemetryData& data) override;

private:
	typedef int(__cdecl* ADLXInitialize_Fn)(uint64_t version, adlx::IADLXSystem** ppSystem);
	typedef int(__cdecl* ADLXTerminate_Fn)();

	HMODULE m_hAdlx;
	bool    m_initialized;

	ADLXInitialize_Fn m_pfnInitialize;
	ADLXTerminate_Fn  m_pfnTerminate;

	adlx::IADLXSystem* m_pSystem;
	adlx::IADLXPerformanceMonitoringServices* m_pPerfMon;
	adlx::IADLXGPU* m_pGpu;
};
