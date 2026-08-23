#include "stdafx.h"
#include "RDOsInfo.h"
#include "RDDiagManager.h"

typedef LONG(WINAPI* RtlGetVersionFunc)(PRTL_OSVERSIONINFOW);

void RDOsInfo::CollectStartup()
{
	std::string osVersion = "Unknown";

	HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
	if (hNtdll) {
		auto pRtlGetVersion = reinterpret_cast<RtlGetVersionFunc>(
			GetProcAddress(hNtdll, "RtlGetVersion"));
		if (pRtlGetVersion) {
			RTL_OSVERSIONINFOW vi = {};
			vi.dwOSVersionInfoSize = sizeof(vi);
			if (pRtlGetVersion(&vi) == 0) {
				DWORD ubr = 0;
				HKEY hKey = NULL;
				if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
					L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
					0, KEY_READ, &hKey) == ERROR_SUCCESS) {
					DWORD size = sizeof(DWORD);
					RegQueryValueExW(hKey, L"UBR", NULL, NULL,
						reinterpret_cast<LPBYTE>(&ubr), &size);
					RegCloseKey(hKey);
				}

				SYSTEM_INFO si = {};
				GetNativeSystemInfo(&si);
				const char* arch = "Unknown";
				switch (si.wProcessorArchitecture) {
				case PROCESSOR_ARCHITECTURE_AMD64: arch = "AMD64"; break;
				case PROCESSOR_ARCHITECTURE_ARM64: arch = "ARM64"; break;
				case PROCESSOR_ARCHITECTURE_INTEL: arch = "x86"; break;
				case PROCESSOR_ARCHITECTURE_ARM:   arch = "ARM"; break;
				}

				char buf[256];
				if (ubr > 0) {
					snprintf(buf, sizeof(buf), "%lu.%lu.%lu.%lu",
						vi.dwMajorVersion, vi.dwMinorVersion, vi.dwBuildNumber, ubr);
				} else {
					snprintf(buf, sizeof(buf), "%lu.%lu.%lu",
						vi.dwMajorVersion, vi.dwMinorVersion, vi.dwBuildNumber);
				}
				osVersion = buf;
			}
		}
	}

	RDDiagManager::SetString(RDMetricId::OsVersion, osVersion.c_str());
}
