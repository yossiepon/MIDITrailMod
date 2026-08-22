#include "stdafx.h"
#include "RDMetricDefs.h"

static const RDMetricMeta s_MetricMetaTable[] =
{
	{ RDMetricId::OsVersion,            "OsVersion",            RDMetricType::String },

	{ RDMetricId::CpuName,              "CpuName",              RDMetricType::String },
	{ RDMetricId::CpuPhysicalCores,     "CpuPhysicalCores",     RDMetricType::Int },
	{ RDMetricId::CpuLogicalProcessors, "CpuLogicalProcessors", RDMetricType::Int },
	{ RDMetricId::CpuUsageSystem,       "CpuUsageSystem",       RDMetricType::Float },
	{ RDMetricId::CpuUsageProcess,      "CpuUsageProcess",      RDMetricType::Float },

	{ RDMetricId::MemoryTotalMB,        "MemoryTotalMB",        RDMetricType::Int },
	{ RDMetricId::MemoryUsedMB,         "MemoryUsedMB",         RDMetricType::Int },
	{ RDMetricId::MemoryAvailableMB,    "MemoryAvailableMB",    RDMetricType::Int },
	{ RDMetricId::MemoryWorkingSetMB,   "MemoryWorkingSetMB",   RDMetricType::Int },
	{ RDMetricId::MemoryHandles,        "MemoryHandles",        RDMetricType::Int },

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
