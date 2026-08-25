#pragma once

#include "RDInterfaces.h"
#include <Windows.h>

struct ExtendedDebugInfo;
struct DebugInfo;

class RDKdmapiInfo : public IRDStartupComponent, public IRDIntervalPollingComponent
{
public:
	RDKdmapiInfo();
	virtual ~RDKdmapiInfo();

	void CollectStartup() override;
	void CollectIntervalPolling() override;
	DWORD GetPollingIntervalMs() const override { return 50; }

private:
	bool _TryDetect();
	bool _ConnectDebugPipe();
	void _DrainPipe();

	typedef ExtendedDebugInfo* (WINAPI *GetModExtendedDebugInfoFunc)();
	typedef DebugInfo* (WINAPI *GetDriverDebugInfoFunc)();

	enum class SynthMode { None, Mod, Standard };

	SynthMode                  m_mode;
	GetModExtendedDebugInfoFunc m_pfnGetModExtendedDebugInfo;
	GetDriverDebugInfoFunc      m_pfnGetDriverDebugInfo;
	HANDLE                     m_hPipe;
};
