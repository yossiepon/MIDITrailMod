#pragma once

#include "RTDiagLib.h"
#include <cstdint>

class RTDIAGLIB_API IRDStartupComponent
{
public:
	virtual ~IRDStartupComponent() = default;
	virtual void CollectStartup() = 0;
};

class RTDIAGLIB_API IRDIntervalPollingComponent
{
public:
	virtual ~IRDIntervalPollingComponent() = default;
	virtual void CollectIntervalPolling() = 0;
	virtual DWORD GetPollingIntervalMs() const = 0;
};

class RTDIAGLIB_API IRDFrameComponent
{
public:
	virtual ~IRDFrameComponent() = default;
	virtual void CollectFrame() = 0;
};
