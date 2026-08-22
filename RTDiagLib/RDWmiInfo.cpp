#include "stdafx.h"
#include "RDWmiInfo.h"
#include "RDDiagManager.h"
#include <spdlog/spdlog.h>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

static std::string BstrToUtf8(BSTR bstr)
{
	if (!bstr) return "";
	int wideLen = static_cast<int>(wcslen(bstr));
	if (wideLen <= 0) return "";
	int utf8Len = WideCharToMultiByte(CP_UTF8, 0, bstr, wideLen, NULL, 0, NULL, NULL);
	if (utf8Len <= 0) return "";
	std::string val(utf8Len, '\0');
	WideCharToMultiByte(CP_UTF8, 0, bstr, wideLen, &val[0], utf8Len, NULL, NULL);
	return val;
}

static std::string QueryWmiString(IWbemServices* pServices, const wchar_t* query, const wchar_t* prop)
{
	IEnumWbemClassObject* pEnum = NULL;
	HRESULT hr = pServices->ExecQuery(
		SysAllocString(L"WQL"), SysAllocString(query),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum);

	std::string result;
	if (SUCCEEDED(hr) && pEnum) {
		IWbemClassObject* pObj = NULL;
		ULONG uReturn = 0;
		if (pEnum->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK && uReturn > 0) {
			VARIANT vtProp;
			VariantInit(&vtProp);
			hr = pObj->Get(prop, 0, &vtProp, NULL, NULL);
			if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR && vtProp.bstrVal) {
				result = BstrToUtf8(vtProp.bstrVal);
			}
			VariantClear(&vtProp);
			pObj->Release();
		}
		pEnum->Release();
	}
	return result;
}

void RDWmiInfo::CollectStartup()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	bool weInitializedCom = SUCCEEDED(hr);

	IWbemLocator* pLocator = NULL;
	hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, reinterpret_cast<void**>(&pLocator));

	if (FAILED(hr)) {
		auto logger = spdlog::get("RD");
		if (logger) logger->warn("WMI: CoCreateInstance failed: 0x{:08X}", static_cast<unsigned>(hr));
		if (weInitializedCom) CoUninitialize();
		return;
	}

	IWbemServices* pServices = NULL;
	hr = pLocator->ConnectServer(
		SysAllocString(L"ROOT\\CIMV2"), NULL, NULL, NULL, 0, NULL, NULL, &pServices);

	if (FAILED(hr)) {
		auto logger = spdlog::get("RD");
		if (logger) logger->warn("WMI: ConnectServer failed: 0x{:08X}", static_cast<unsigned>(hr));
		pLocator->Release();
		if (weInitializedCom) CoUninitialize();
		return;
	}

	hr = CoSetProxyBlanket(pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

	// OS name (e.g. "Microsoft Windows 11 Pro")
	std::string osCaption = QueryWmiString(pServices,
		L"SELECT Caption FROM Win32_OperatingSystem", L"Caption");
	if (!osCaption.empty()) {
		RDDiagManager::SetString(RDMetricId::OsProductName, osCaption.c_str());
	}

	// GPU driver version (match by GPU name from DXGI)
	// Requires RDGpuInfo to have run first (GpuName must be set)
	{
		std::string activeGpuName = RDDiagManager::GetString(RDMetricId::GpuName);

		IEnumWbemClassObject* pEnum = NULL;
		hr = pServices->ExecQuery(
			SysAllocString(L"WQL"),
			SysAllocString(L"SELECT Name, DriverVersion FROM Win32_VideoController"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum);

		if (SUCCEEDED(hr) && pEnum) {
			IWbemClassObject* pObj = NULL;
			ULONG uReturn = 0;
			std::string fallbackDriver;
			bool matched = false;

			while (pEnum->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK && uReturn > 0) {
				VARIANT vtName, vtDriver;
				VariantInit(&vtName);
				VariantInit(&vtDriver);

				HRESULT hrName = pObj->Get(L"Name", 0, &vtName, NULL, NULL);
				HRESULT hrDriver = pObj->Get(L"DriverVersion", 0, &vtDriver, NULL, NULL);

				std::string name = (SUCCEEDED(hrName) && vtName.vt == VT_BSTR) ?
					BstrToUtf8(vtName.bstrVal) : "";
				std::string driver = (SUCCEEDED(hrDriver) && vtDriver.vt == VT_BSTR) ?
					BstrToUtf8(vtDriver.bstrVal) : "";

				if (fallbackDriver.empty() && !driver.empty()) {
					fallbackDriver = driver;
				}

				if (!activeGpuName.empty() && !name.empty() &&
					name.find(activeGpuName) != std::string::npos) {
					RDDiagManager::SetString(RDMetricId::GpuDriverVersion, driver.c_str());
					matched = true;
				}

				VariantClear(&vtName);
				VariantClear(&vtDriver);
				pObj->Release();

				if (matched) break;
			}

			if (!matched && !fallbackDriver.empty()) {
				RDDiagManager::SetString(RDMetricId::GpuDriverVersion, fallbackDriver.c_str());
			}

			pEnum->Release();
		}
	}

	// Machine type
	{
		IEnumWbemClassObject* pEnum = NULL;
		hr = pServices->ExecQuery(
			SysAllocString(L"WQL"),
			SysAllocString(L"SELECT ChassisTypes FROM Win32_SystemEnclosure"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum);

		if (SUCCEEDED(hr) && pEnum) {
			IWbemClassObject* pObj = NULL;
			ULONG uReturn = 0;
			if (pEnum->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK && uReturn > 0) {
				VARIANT vtProp;
				VariantInit(&vtProp);
				hr = pObj->Get(L"ChassisTypes", 0, &vtProp, NULL, NULL);
				if (SUCCEEDED(hr) && (vtProp.vt & VT_ARRAY)) {
					SAFEARRAY* pArray = vtProp.parray;
					if (pArray) {
						LONG lBound = 0, uBound = 0;
						SafeArrayGetLBound(pArray, 1, &lBound);
						SafeArrayGetUBound(pArray, 1, &uBound);
						if (lBound <= uBound) {
							int chassisType = 0;
							VARTYPE baseVt = vtProp.vt & ~VT_ARRAY;
							if (baseVt == VT_I4) {
								int val = 0;
								SafeArrayGetElement(pArray, &lBound, &val);
								chassisType = val;
							} else if (baseVt == VT_I2) {
								short val = 0;
								SafeArrayGetElement(pArray, &lBound, &val);
								chassisType = val;
							} else if (baseVt == VT_VARIANT) {
								VARIANT elem;
								VariantInit(&elem);
								SafeArrayGetElement(pArray, &lBound, &elem);
								if (elem.vt == VT_I4) chassisType = elem.lVal;
								else if (elem.vt == VT_I2) chassisType = elem.iVal;
								VariantClear(&elem);
							}

							const char* typeName = "Unknown";
							switch (chassisType) {
							case 1:  typeName = "Other"; break;
							case 3:  typeName = "Desktop"; break;
							case 4:  typeName = "Low Profile Desktop"; break;
							case 5:  typeName = "Pizza Box"; break;
							case 6:  typeName = "Mini Tower"; break;
							case 7:  typeName = "Tower"; break;
							case 8:  typeName = "Portable"; break;
							case 9:  typeName = "Laptop"; break;
							case 10: typeName = "Notebook"; break;
							case 11: typeName = "Hand Held"; break;
							case 13: typeName = "All in One"; break;
							case 14: typeName = "Sub Notebook"; break;
							case 15: typeName = "Space-Saving"; break;
							case 23: typeName = "Rack Mount"; break;
							case 24: typeName = "Sealed-Case PC"; break;
							case 30: typeName = "Tablet"; break;
							case 31: typeName = "Convertible"; break;
							case 32: typeName = "Detachable"; break;
							case 35: typeName = "Mini PC"; break;
							case 36: typeName = "Stick PC"; break;
							}
							RDDiagManager::SetString(RDMetricId::MachineType, typeName);
						}
					}
				}
				VariantClear(&vtProp);
				pObj->Release();
			}
			pEnum->Release();
		}
	}

	pServices->Release();
	pLocator->Release();

	if (weInitializedCom) {
		CoUninitialize();
	}
}
