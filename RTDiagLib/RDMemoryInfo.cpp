#include "stdafx.h"
#include "RDMemoryInfo.h"
#include "RDDiagManager.h"

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
	}
}
