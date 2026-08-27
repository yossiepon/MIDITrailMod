//******************************************************************************
//
// RTDiagLib / RDMemoryInfo
//
// System memory and process memory collector.
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
//******************************************************************************

#include "stdafx.h"
#include "RDMemoryInfo.h"
#include "RDDiagManager.h"
#include <psapi.h>

void RDMemoryInfo::CollectStartup()
{
	MEMORYSTATUSEX memStatus = {};
	memStatus.dwLength = sizeof(memStatus);

	if (GlobalMemoryStatusEx(&memStatus)) {
		int64_t physTotalMB = static_cast<int64_t>(memStatus.ullTotalPhys / (1024 * 1024));
		int64_t physAvailMB = static_cast<int64_t>(memStatus.ullAvailPhys / (1024 * 1024));
		RDDiagManager::SetInt(RDMetricId::PhysMemTotalMB, physTotalMB);
		RDDiagManager::SetInt(RDMetricId::PhysMemAvailableMB, physAvailMB);

		int64_t commitLimitMB = static_cast<int64_t>(memStatus.ullTotalPageFile / (1024 * 1024));
		int64_t commitAvailMB = static_cast<int64_t>(memStatus.ullAvailPageFile / (1024 * 1024));
		int64_t commitUsedMB = commitLimitMB - commitAvailMB;
		RDDiagManager::SetInt(RDMetricId::CommitLimitMB, commitLimitMB);
		RDDiagManager::SetInt(RDMetricId::CommitUsedMB, commitUsedMB);
		RDDiagManager::SetInt(RDMetricId::CommitFreeMB, commitAvailMB);
	}
}

void RDMemoryInfo::CollectIntervalPolling()
{
	MEMORYSTATUSEX memStatus = {};
	memStatus.dwLength = sizeof(memStatus);

	if (GlobalMemoryStatusEx(&memStatus)) {
		int64_t physAvailMB = static_cast<int64_t>(memStatus.ullAvailPhys / (1024 * 1024));
		RDDiagManager::SetInt(RDMetricId::PhysMemAvailableMB, physAvailMB);

		int64_t commitLimitMB = static_cast<int64_t>(memStatus.ullTotalPageFile / (1024 * 1024));
		int64_t commitAvailMB = static_cast<int64_t>(memStatus.ullAvailPageFile / (1024 * 1024));
		int64_t commitUsedMB = commitLimitMB - commitAvailMB;
		RDDiagManager::SetInt(RDMetricId::CommitUsedMB, commitUsedMB);
		RDDiagManager::SetInt(RDMetricId::CommitFreeMB, commitAvailMB);
	}

	PROCESS_MEMORY_COUNTERS_EX pmcEx = {};
	pmcEx.cb = sizeof(pmcEx);
	if (GetProcessMemoryInfo(GetCurrentProcess(),
		reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmcEx), sizeof(pmcEx))) {
		int64_t commitMB = static_cast<int64_t>(pmcEx.PrivateUsage / (1024 * 1024));
		int64_t wsMB = static_cast<int64_t>(pmcEx.WorkingSetSize / (1024 * 1024));
		RDDiagManager::SetInt(RDMetricId::ProcessCommitMB, commitMB);
		RDDiagManager::SetInt(RDMetricId::ProcessWorkingSetMB, wsMB);
	}

	DWORD handleCount = 0;
	if (GetProcessHandleCount(GetCurrentProcess(), &handleCount)) {
		RDDiagManager::SetInt(RDMetricId::ProcessHandles, static_cast<int64_t>(handleCount));
	}
}
