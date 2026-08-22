#include "stdafx.h"
#include "RDCpuInfo.h"
#include "RDDiagManager.h"
#include <intrin.h>
#include <string>

void RDCpuInfo::CollectStartup()
{
	int cpuInfo[4] = {};

	// CPU vendor string via __cpuid leaf 0 (EBX+EDX+ECX)
	__cpuid(cpuInfo, 0);
	char vendor[13] = {};
	memcpy(vendor + 0, &cpuInfo[1], 4);  // EBX
	memcpy(vendor + 4, &cpuInfo[3], 4);  // EDX
	memcpy(vendor + 8, &cpuInfo[2], 4);  // ECX
	vendor[12] = '\0';
	RDDiagManager::SetString(RDMetricId::CpuVendor, vendor);

	// CPU brand string via __cpuid (leaves 0x80000002-0x80000004)
	char brand[49] = {};
	__cpuid(cpuInfo, 0x80000000);
	unsigned int maxExtFunc = static_cast<unsigned int>(cpuInfo[0]);

	if (maxExtFunc >= 0x80000004) {
		__cpuid(cpuInfo, 0x80000002);
		memcpy(brand + 0, cpuInfo, sizeof(cpuInfo));
		__cpuid(cpuInfo, 0x80000003);
		memcpy(brand + 16, cpuInfo, sizeof(cpuInfo));
		__cpuid(cpuInfo, 0x80000004);
		memcpy(brand + 32, cpuInfo, sizeof(cpuInfo));
		brand[48] = '\0';
	}

	std::string cpuName(brand);
	auto start = cpuName.find_first_not_of(' ');
	auto end = cpuName.find_last_not_of(' ');
	if (start != std::string::npos) {
		cpuName = cpuName.substr(start, end - start + 1);
	}
	RDDiagManager::SetString(RDMetricId::CpuName, cpuName.c_str());

	// Processor architecture
	SYSTEM_INFO si = {};
	GetNativeSystemInfo(&si);
	const char* arch = "Unknown";
	switch (si.wProcessorArchitecture) {
	case PROCESSOR_ARCHITECTURE_AMD64: arch = "AMD64"; break;
	case PROCESSOR_ARCHITECTURE_ARM64: arch = "ARM64"; break;
	case PROCESSOR_ARCHITECTURE_INTEL: arch = "x86"; break;
	case PROCESSOR_ARCHITECTURE_ARM:   arch = "ARM"; break;
	}
	RDDiagManager::SetString(RDMetricId::CpuArchitecture, arch);

	// Core/thread count via GetLogicalProcessorInformationEx
	int physicalCores = 0;
	int logicalProcessors = 0;

	DWORD bufferSize = 0;
	GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &bufferSize);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && bufferSize > 0) {
		std::vector<uint8_t> buffer(bufferSize);
		if (GetLogicalProcessorInformationEx(RelationProcessorCore,
			reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()),
			&bufferSize)) {
			DWORD offset = 0;
			while (offset < bufferSize) {
				auto* info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
					buffer.data() + offset);
				if (info->Relationship == RelationProcessorCore) {
					physicalCores++;
					for (WORD g = 0; g < info->Processor.GroupCount; g++) {
						KAFFINITY mask = info->Processor.GroupMask[g].Mask;
						while (mask) {
							logicalProcessors += (mask & 1) ? 1 : 0;
							mask >>= 1;
						}
					}
				}
				offset += info->Size;
			}
		}
	}

	if (physicalCores == 0) {
		logicalProcessors = static_cast<int>(si.dwNumberOfProcessors);
		physicalCores = logicalProcessors;
	}

	// Socket (package) count
	int sockets = 0;
	bufferSize = 0;
	GetLogicalProcessorInformationEx(RelationProcessorPackage, nullptr, &bufferSize);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && bufferSize > 0) {
		std::vector<uint8_t> buffer(bufferSize);
		if (GetLogicalProcessorInformationEx(RelationProcessorPackage,
			reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()),
			&bufferSize)) {
			DWORD offset = 0;
			while (offset < bufferSize) {
				auto* info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(
					buffer.data() + offset);
				if (info->Relationship == RelationProcessorPackage) {
					sockets++;
				}
				offset += info->Size;
			}
		}
	}
	if (sockets == 0) sockets = 1;

	RDDiagManager::SetInt(RDMetricId::CpuSockets, sockets);
	RDDiagManager::SetInt(RDMetricId::CpuPhysicalCores, physicalCores);
	RDDiagManager::SetInt(RDMetricId::CpuLogicalProcessors, logicalProcessors);

	// Base clock from registry
	HKEY hKey = NULL;
	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
		L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
		0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		DWORD mhz = 0;
		DWORD size = sizeof(DWORD);
		if (RegQueryValueExW(hKey, L"~MHz", NULL, NULL,
			reinterpret_cast<LPBYTE>(&mhz), &size) == ERROR_SUCCESS) {
			RDDiagManager::SetInt(RDMetricId::CpuBaseMHz, static_cast<int64_t>(mhz));
		}
		RegCloseKey(hKey);
	}
}
