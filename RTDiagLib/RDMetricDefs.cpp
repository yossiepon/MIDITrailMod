#include "stdafx.h"
#include "RDMetricDefs.h"

static const RDMetricMeta s_MetricMetaTable[] =
{
	{ RDMetricId::OsProductName,        "OsProductName",        RDMetricType::String },
	{ RDMetricId::OsVersion,            "OsVersion",            RDMetricType::String },

	{ RDMetricId::CpuName,              "CpuName",              RDMetricType::String },
	{ RDMetricId::CpuArchitecture,      "CpuArchitecture",      RDMetricType::String },
	{ RDMetricId::CpuVendor,            "CpuVendor",            RDMetricType::String },
	{ RDMetricId::CpuSockets,           "CpuSockets",           RDMetricType::Int },
	{ RDMetricId::CpuPhysicalCores,     "CpuPhysicalCores",     RDMetricType::Int },
	{ RDMetricId::CpuLogicalProcessors, "CpuLogicalProcessors", RDMetricType::Int },
	{ RDMetricId::CpuBaseMHz,           "CpuBaseMHz",           RDMetricType::Int },
	{ RDMetricId::CpuUsageSystem,       "CpuUsageSystem",       RDMetricType::Float },
	{ RDMetricId::CpuUsageProcess,      "CpuUsageProcess",      RDMetricType::Float },

	{ RDMetricId::PhysMemTotalMB,       "PhysMemTotalMB",       RDMetricType::Int },
	{ RDMetricId::PhysMemAvailableMB,   "PhysMemAvailableMB",   RDMetricType::Int },

	{ RDMetricId::CommitLimitMB,        "CommitLimitMB",        RDMetricType::Int },
	{ RDMetricId::CommitUsedMB,         "CommitUsedMB",         RDMetricType::Int },

	{ RDMetricId::ProcessWorkingSetMB,  "ProcessWorkingSetMB",  RDMetricType::Int },
	{ RDMetricId::ProcessHandles,       "ProcessHandles",       RDMetricType::Int },

	{ RDMetricId::GpuName,              "GpuName",              RDMetricType::String },
	{ RDMetricId::GpuVramTotalMB,       "GpuVramTotalMB",       RDMetricType::Int },
	{ RDMetricId::GpuDriverVersion,     "GpuDriverVersion",     RDMetricType::String },
	{ RDMetricId::GpuVramUsedMB,        "GpuVramUsedMB",        RDMetricType::Int },
	{ RDMetricId::GpuVramFreeMB,        "GpuVramFreeMB",        RDMetricType::Int },
	{ RDMetricId::GpuVramBudgetMB,      "GpuVramBudgetMB",      RDMetricType::Int },
	{ RDMetricId::GpuUsagePercent,      "GpuUsagePercent",       RDMetricType::Float },

	{ RDMetricId::MachineType,          "MachineType",          RDMetricType::String },

	{ RDMetricId::AppSceneUpdateTimeMs, "AppSceneUpdateTimeMs", RDMetricType::Float },
	{ RDMetricId::AppDrawTimeMs,        "AppDrawTimeMs",        RDMetricType::Float },
	{ RDMetricId::AppPresentTimeMs,     "AppPresentTimeMs",     RDMetricType::Float },
};

static_assert(
	sizeof(s_MetricMetaTable) / sizeof(s_MetricMetaTable[0]) == static_cast<size_t>(RDMetricId::COUNT),
	"s_MetricMetaTable must have exactly RDMetricId::COUNT entries"
);

const RDMetricMeta* RDGetMetricMetaTable()
{
	return s_MetricMetaTable;
}

size_t RDGetMetricMetaCount()
{
	return sizeof(s_MetricMetaTable) / sizeof(s_MetricMetaTable[0]);
}
