#pragma once

#include "RTDiagLib.h"
#include "RDMetricDefs.h"
#include "RDInterfaces.h"
#include "RDGpuTimestamp.h"
#include <d3d11.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

class RTDIAGLIB_API RDDiagManager
{
public:
	static int Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext = nullptr);
	static void Update();
	static void Terminate();

	static double GetFloat(RDMetricId id);
	static int64_t GetInt(RDMetricId id);
	static const char* GetString(RDMetricId id);

	static void SetFloat(RDMetricId id, double value);
	static void SetInt(RDMetricId id, int64_t value);
	static void SetString(RDMetricId id, const char* value);

	static std::vector<RDFormattedEntry> Format(
		const RDFormatTemplateEntry* pProfile, size_t count);

	static void RegisterStartupComponent(IRDStartupComponent* pComponent);
	static void RegisterIntervalPollingComponent(IRDIntervalPollingComponent* pComponent);
	static void RegisterFrameComponent(IRDFrameComponent* pComponent);

	static void GpuTimestampBeginFrame();
	static void GpuTimestampEndFrame();

	static void ResetFrameMetrics();

private:
	RDDiagManager() = delete;
	~RDDiagManager() = delete;

	struct MetricValue
	{
		int64_t     intVal;
		double      floatVal;
		std::string strVal;
	};

	struct IntervalEntry
	{
		IRDIntervalPollingComponent* pComponent;
		LARGE_INTEGER lastCollected;
	};

	struct CompiledSegment
	{
		enum Kind { Literal, Metric } kind;
		std::string literal;
		RDMetricId  metricId;
		RDMetricType metricType;
	};

	struct CompiledTemplate
	{
		std::string label;
		std::vector<CompiledSegment> segments;
	};

	static void _CompileTemplate(
		const char* templateStr,
		std::vector<CompiledSegment>& outSegments);

	static std::string _FormatCompiledTemplate(
		const std::vector<CompiledSegment>& segments);

	static void _BuildKeyToIdMap();

	static bool s_isInitialized;
	static MetricValue s_metrics[static_cast<size_t>(RDMetricId::COUNT)];

	static std::vector<IRDStartupComponent*> s_startupComponents;
	static std::vector<IntervalEntry> s_intervalComponents;
	static std::vector<IRDFrameComponent*> s_frameComponents;

	static std::unordered_map<std::string, size_t> s_keyToIdMap;

	static std::vector<std::string> s_formatBuffer;
	static LARGE_INTEGER s_perfFrequency;

	static std::unordered_map<const RDFormatTemplateEntry*, std::vector<CompiledTemplate>>
		s_compiledProfiles;

	static std::vector<std::function<void()>> s_componentDeleters;

	static ID3D11DeviceContext* s_pDeviceContext;
	static RDGpuTimestamp s_gpuTimestamp;

	static LARGE_INTEGER s_lastLogTime;
	static const DWORD LOG_INTERVAL_MS = 10000;
};
