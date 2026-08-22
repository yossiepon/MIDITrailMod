#pragma once

#include "RDMetricDefs.h"

namespace RDFormatProfile
{
	static const RDFormatTemplateEntry MachineSignature[] = {
		{ "OS",      "${OsProductName} (${OsVersion})" },
		{ "CPU",     "${CpuName} (${CpuArchitecture})" },
		{ "CPU info","${CpuVendor}, ${CpuSockets}P/${CpuPhysicalCores}C/${CpuLogicalProcessors}T, ${CpuBaseMHz} MHz" },
		{ "GPU",     "${GpuName}" },
		{ "VRAM",    "${GpuVramUsedMB} / ${GpuVramTotalMB} MB used (Budget: ${GpuVramBudgetMB} MB)" },
		{ "Driver",  "${GpuDriverVersion}" },
		{ "RAM",     "${PhysMemAvailableMB} / ${PhysMemTotalMB} MB free" },
		{ "Commit",  "${CommitUsedMB} / ${CommitLimitMB} MB used" },
		{ "Machine", "${MachineType}" },
	};

	static const size_t MachineSignatureCount =
		sizeof(MachineSignature) / sizeof(MachineSignature[0]);
}
