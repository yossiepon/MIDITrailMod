#pragma once

#include "RDInterfaces.h"

class RDCpuInfo : public IRDStartupComponent, public IRDIntervalPollingComponent
{
public:
	RDCpuInfo();
	virtual ~RDCpuInfo() = default;

	void CollectStartup() override;
	void CollectIntervalPolling() override;
	DWORD GetPollingIntervalMs() const override { return 1000; }

private:
	ULARGE_INTEGER m_prevIdleTime;
	ULARGE_INTEGER m_prevKernelTime;
	ULARGE_INTEGER m_prevUserTime;
	ULARGE_INTEGER m_prevProcKernelTime;
	ULARGE_INTEGER m_prevProcUserTime;
	LARGE_INTEGER  m_prevProcWallTime;
	int m_logicalProcessorCount;
};
