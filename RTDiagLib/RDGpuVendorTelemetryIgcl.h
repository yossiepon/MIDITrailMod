//******************************************************************************
//
// RTDiagLib / RDGpuVendorTelemetryIgcl
//
// Intel GPU vendor telemetry provider via IGCL (64bit only).
//
// Copyright (C) 2026 Yossiepon Oniichan. All Rights Reserved.
//
// Struct definitions derived from:
//   Intel Graphics Control Library SDK - https://github.com/intel/drivers.gpu.control-library
//
//******************************************************************************

#pragma once

#include "IRDGpuVendorTelemetryProvider.h"

#ifdef _M_AMD64

#include <Windows.h>

class RDGpuVendorTelemetryIgclProvider : public IRDGpuVendorTelemetryProvider
{
public:
	RDGpuVendorTelemetryIgclProvider();
	~RDGpuVendorTelemetryIgclProvider() override;

	bool Initialize() override;
	void Shutdown() override;
	void CollectTelemetry(RDGpuVendorTelemetryData& data) override;

private:
	typedef uint32_t ctl_result_t;
	static const ctl_result_t CTL_RESULT_SUCCESS = 0;

	typedef struct _ctl_api_handle_t* ctl_api_handle_t;
	typedef struct _ctl_device_adapter_handle_t* ctl_device_adapter_handle_t;

	typedef uint32_t ctl_init_flags_t;
	typedef uint32_t ctl_version_info_t;

	struct ctl_init_args_t {
		uint32_t Size;
		uint8_t Version;
		ctl_version_info_t AppVersion;
		ctl_init_flags_t flags;
		ctl_version_info_t SupportedVersion;
		uint8_t ApplicationUID[16];
	};

	enum ctl_units_t : uint32_t {
		CTL_UNITS_FREQUENCY_MHZ = 0,
		CTL_UNITS_VOLTAGE_VOLTS = 3,
		CTL_UNITS_POWER_WATTS = 4,
		CTL_UNITS_TEMPERATURE_CELSIUS = 5,
		CTL_UNITS_ENERGY_JOULES = 7,
		CTL_UNITS_ANGULAR_SPEED_RPM = 9,
		CTL_UNITS_TIME_SECONDS = 10,
		CTL_UNITS_PERCENT = 11,
	};

	enum ctl_data_type_t : uint32_t {
		CTL_DATA_TYPE_INT8 = 0,
		CTL_DATA_TYPE_UINT8 = 1,
		CTL_DATA_TYPE_INT16 = 2,
		CTL_DATA_TYPE_UINT16 = 3,
		CTL_DATA_TYPE_INT32 = 4,
		CTL_DATA_TYPE_UINT32 = 5,
		CTL_DATA_TYPE_INT64 = 6,
		CTL_DATA_TYPE_UINT64 = 7,
		CTL_DATA_TYPE_FLOAT = 8,
		CTL_DATA_TYPE_DOUBLE = 9,
	};

	union ctl_data_value_t {
		int8_t data8;
		uint8_t datau8;
		int16_t data16;
		uint16_t datau16;
		int32_t data32;
		uint32_t datau32;
		int64_t data64;
		uint64_t datau64;
		float datafloat;
		double datadouble;
	};

	struct ctl_oc_telemetry_item_t {
		bool bSupported;
		ctl_units_t units;
		ctl_data_type_t type;
		ctl_data_value_t value;
	};

	enum ctl_psu_type_t : uint32_t { CTL_PSU_TYPE_PSU_NONE = 0 };

	struct ctl_psu_info_t {
		bool bSupported;
		ctl_psu_type_t psuType;
		ctl_oc_telemetry_item_t energyCounter;
		ctl_oc_telemetry_item_t voltage;
	};

	static const int CTL_PSU_COUNT = 5;
	static const int CTL_FAN_COUNT = 5;

	struct ctl_power_telemetry_t {
		uint32_t Size;
		uint8_t Version;
		ctl_oc_telemetry_item_t timeStamp;
		ctl_oc_telemetry_item_t gpuEnergyCounter;
		ctl_oc_telemetry_item_t gpuVoltage;
		ctl_oc_telemetry_item_t gpuCurrentClockFrequency;
		ctl_oc_telemetry_item_t gpuCurrentTemperature;
		ctl_oc_telemetry_item_t globalActivityCounter;
		ctl_oc_telemetry_item_t renderComputeActivityCounter;
		ctl_oc_telemetry_item_t mediaActivityCounter;
		bool gpuPowerLimited;
		bool gpuTemperatureLimited;
		bool gpuCurrentLimited;
		bool gpuVoltageLimited;
		bool gpuUtilizationLimited;
		ctl_oc_telemetry_item_t vramEnergyCounter;
		ctl_oc_telemetry_item_t vramVoltage;
		ctl_oc_telemetry_item_t vramCurrentClockFrequency;
		ctl_oc_telemetry_item_t vramCurrentEffectiveFrequency;
		ctl_oc_telemetry_item_t vramReadBandwidthCounter;
		ctl_oc_telemetry_item_t vramWriteBandwidthCounter;
		ctl_oc_telemetry_item_t vramCurrentTemperature;
		bool vramPowerLimited;
		bool vramTemperatureLimited;
		bool vramCurrentLimited;
		bool vramVoltageLimited;
		bool vramUtilizationLimited;
		ctl_oc_telemetry_item_t totalCardEnergyCounter;
		ctl_psu_info_t psu[CTL_PSU_COUNT];
		ctl_oc_telemetry_item_t fanSpeed[CTL_FAN_COUNT];
		ctl_oc_telemetry_item_t gpuVrTemp;
		ctl_oc_telemetry_item_t vramVrTemp;
		ctl_oc_telemetry_item_t saVrTemp;
		ctl_oc_telemetry_item_t gpuEffectiveClock;
		ctl_oc_telemetry_item_t gpuOverVoltagePercent;
		ctl_oc_telemetry_item_t gpuPowerPercent;
		ctl_oc_telemetry_item_t gpuTemperaturePercent;
		ctl_oc_telemetry_item_t vramReadBandwidth;
		ctl_oc_telemetry_item_t vramWriteBandwidth;
	};

	typedef ctl_result_t(__cdecl* ctlInitFunc)(ctl_init_args_t*, ctl_api_handle_t*);
	typedef ctl_result_t(__cdecl* ctlCloseFunc)(ctl_api_handle_t);
	typedef ctl_result_t(__cdecl* ctlEnumerateDevicesFunc)(ctl_api_handle_t, uint32_t*, ctl_device_adapter_handle_t*);
	typedef ctl_result_t(__cdecl* ctlPowerTelemetryGetFunc)(ctl_device_adapter_handle_t, ctl_power_telemetry_t*);

	double _ReadItemDouble(const ctl_oc_telemetry_item_t& item);

	HMODULE m_hIgcl;
	bool    m_initialized;

	ctl_api_handle_t            m_apiHandle;
	ctl_device_adapter_handle_t m_deviceHandle;

	ctlInitFunc              m_pfnInit;
	ctlCloseFunc             m_pfnClose;
	ctlEnumerateDevicesFunc  m_pfnEnumerateDevices;
	ctlPowerTelemetryGetFunc m_pfnPowerTelemetryGet;

	ctl_oc_telemetry_item_t m_prevEnergyCounter;
	ctl_oc_telemetry_item_t m_prevEnergyTimestamp;
	ctl_oc_telemetry_item_t m_prevGlobalActivity;
	ctl_oc_telemetry_item_t m_prevActivityTimestamp;
	bool m_hasPrevEnergy;
	bool m_hasPrevActivity;
};

#endif // _M_AMD64
