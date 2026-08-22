#include "stdafx.h"
#include "RDDiagManager.h"
#include "RDOsInfo.h"
#include "RDCpuInfo.h"
#include "RDGpuInfo.h"
#include "RDMemoryInfo.h"
#include "RDWmiInfo.h"
#include "RDFormatProfiles.h"
#include <spdlog/spdlog.h>
#include <cassert>

extern const RDMetricMeta* RDGetMetricMetaTable();
extern size_t RDGetMetricMetaCount();

bool RDDiagManager::s_isInitialized = false;
RDDiagManager::MetricValue RDDiagManager::s_metrics[static_cast<size_t>(RDMetricId::COUNT)];

std::vector<IRDStartupComponent*> RDDiagManager::s_startupComponents;
std::vector<RDDiagManager::IntervalEntry> RDDiagManager::s_intervalComponents;
std::vector<IRDFrameComponent*> RDDiagManager::s_frameComponents;

std::unordered_map<std::string, size_t> RDDiagManager::s_keyToIdMap;

std::vector<std::string> RDDiagManager::s_formatBuffer;
LARGE_INTEGER RDDiagManager::s_perfFrequency = {};

std::unordered_map<const RDFormatTemplateEntry*, std::vector<RDDiagManager::CompiledTemplate>>
	RDDiagManager::s_compiledProfiles;

std::vector<std::unique_ptr<IRDStartupComponent>> RDDiagManager::s_ownedStartup;
std::vector<std::unique_ptr<IRDIntervalPollingComponent>> RDDiagManager::s_ownedInterval;
std::vector<std::unique_ptr<IRDFrameComponent>> RDDiagManager::s_ownedFrame;

int RDDiagManager::Initialize(ID3D11Device* pDevice)
{
	if (s_isInitialized) {
		return 0;
	}

	QueryPerformanceFrequency(&s_perfFrequency);
	for (auto& m : s_metrics) {
		m.intVal = 0;
		m.floatVal = 0.0;
		m.strVal.clear();
	}

	_BuildKeyToIdMap();

	// Phase 1A startup components
	// Order matters: RDWmiInfo reads GpuName (set by RDGpuInfo) to match driver version
	{
		auto osInfo = std::make_unique<RDOsInfo>();
		RegisterStartupComponent(osInfo.get());
		s_ownedStartup.push_back(std::move(osInfo));

		auto cpuInfo = std::make_unique<RDCpuInfo>();
		RegisterStartupComponent(cpuInfo.get());
		s_ownedStartup.push_back(std::move(cpuInfo));

		auto gpuInfo = std::make_unique<RDGpuInfo>();
		gpuInfo->SetDevice(pDevice);
		RegisterStartupComponent(gpuInfo.get());
		s_ownedStartup.push_back(std::move(gpuInfo));

		auto memInfo = std::make_unique<RDMemoryInfo>();
		RegisterStartupComponent(memInfo.get());
		s_ownedStartup.push_back(std::move(memInfo));

		auto wmiInfo = std::make_unique<RDWmiInfo>();
		RegisterStartupComponent(wmiInfo.get());
		s_ownedStartup.push_back(std::move(wmiInfo));
	}

	for (auto* pComp : s_startupComponents) {
		pComp->CollectStartup();
	}

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	for (auto& entry : s_intervalComponents) {
		entry.pComponent->CollectIntervalPolling();
		entry.lastCollected = now;
	}

	s_isInitialized = true;

	auto logger = spdlog::get("RD");
	if (logger) {
		logger->info("RDDiagManager initialized");

		auto signature = Format(
			RDFormatProfile::MachineSignature,
			RDFormatProfile::MachineSignatureCount);
		for (const auto& entry : signature) {
			logger->info("{}: {}", entry.label, entry.value);
		}
	}

	return 0;
}

void RDDiagManager::Update()
{
	if (!s_isInitialized) {
		return;
	}

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	for (auto& entry : s_intervalComponents) {
		double elapsedMs = static_cast<double>(now.QuadPart - entry.lastCollected.QuadPart)
			* 1000.0 / static_cast<double>(s_perfFrequency.QuadPart);

		if (elapsedMs >= static_cast<double>(entry.pComponent->GetPollingIntervalMs())) {
			entry.pComponent->CollectIntervalPolling();
			entry.lastCollected = now;
		}
	}

	for (auto* pComp : s_frameComponents) {
		pComp->CollectFrame();
	}
}

void RDDiagManager::Terminate()
{
	if (!s_isInitialized) {
		return;
	}

	auto logger = spdlog::get("RD");
	if (logger) {
		logger->info("RDDiagManager terminated");
	}

	s_startupComponents.clear();
	s_intervalComponents.clear();
	s_frameComponents.clear();
	s_ownedStartup.clear();
	s_ownedInterval.clear();
	s_ownedFrame.clear();
	s_keyToIdMap.clear();
	s_formatBuffer.clear();
	s_compiledProfiles.clear();

	for (auto& m : s_metrics) {
		m.intVal = 0;
		m.floatVal = 0.0;
		m.strVal.clear();
	}
	s_isInitialized = false;
}

double RDDiagManager::GetFloat(RDMetricId id)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	return s_metrics[static_cast<size_t>(id)].floatVal;
}

int64_t RDDiagManager::GetInt(RDMetricId id)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	return s_metrics[static_cast<size_t>(id)].intVal;
}

const char* RDDiagManager::GetString(RDMetricId id)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	return s_metrics[static_cast<size_t>(id)].strVal.c_str();
}

void RDDiagManager::SetFloat(RDMetricId id, double value)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	s_metrics[static_cast<size_t>(id)].floatVal = value;
}

void RDDiagManager::SetInt(RDMetricId id, int64_t value)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	s_metrics[static_cast<size_t>(id)].intVal = value;
}

void RDDiagManager::SetString(RDMetricId id, const char* value)
{
	assert(static_cast<size_t>(id) < static_cast<size_t>(RDMetricId::COUNT));
	s_metrics[static_cast<size_t>(id)].strVal = value ? value : "";
}

void RDDiagManager::RegisterStartupComponent(IRDStartupComponent* pComponent)
{
	s_startupComponents.push_back(pComponent);
}

void RDDiagManager::RegisterIntervalPollingComponent(IRDIntervalPollingComponent* pComponent)
{
	IntervalEntry entry;
	entry.pComponent = pComponent;
	entry.lastCollected.QuadPart = 0;
	s_intervalComponents.push_back(entry);
}

void RDDiagManager::RegisterFrameComponent(IRDFrameComponent* pComponent)
{
	s_frameComponents.push_back(pComponent);
}

std::vector<RDFormattedEntry> RDDiagManager::Format(
	const RDFormatTemplateEntry* pProfile, size_t count)
{
	auto it = s_compiledProfiles.find(pProfile);
	if (it == s_compiledProfiles.end()) {
		std::vector<CompiledTemplate> compiled;
		compiled.resize(count);
		for (size_t i = 0; i < count; ++i) {
			compiled[i].label = pProfile[i].label;
			_CompileTemplate(pProfile[i].templateStr, compiled[i].segments);
		}
		it = s_compiledProfiles.emplace(pProfile, std::move(compiled)).first;
	}

	const auto& compiled = it->second;
	s_formatBuffer.clear();
	s_formatBuffer.reserve(compiled.size());

	std::vector<RDFormattedEntry> result;
	result.reserve(compiled.size());

	for (const auto& tmpl : compiled) {
		s_formatBuffer.push_back(_FormatCompiledTemplate(tmpl.segments));
		RDFormattedEntry entry;
		entry.label = tmpl.label.c_str();
		entry.value = s_formatBuffer.back().c_str();
		result.push_back(entry);
	}

	return result;
}

void RDDiagManager::_BuildKeyToIdMap()
{
	const RDMetricMeta* table = RDGetMetricMetaTable();
	size_t count = RDGetMetricMetaCount();
	for (size_t i = 0; i < count; ++i) {
		s_keyToIdMap[table[i].key] = static_cast<size_t>(table[i].id);
	}
}

void RDDiagManager::_CompileTemplate(
	const char* templateStr,
	std::vector<CompiledSegment>& outSegments)
{
	outSegments.clear();
	const char* p = templateStr;
	std::string literal;

	const RDMetricMeta* metaTable = RDGetMetricMetaTable();

	while (*p) {
		if (p[0] == '$' && p[1] == '{') {
			if (!literal.empty()) {
				CompiledSegment seg;
				seg.kind = CompiledSegment::Literal;
				seg.literal = std::move(literal);
				seg.metricId = RDMetricId::COUNT;
				seg.metricType = RDMetricType::Int;
				outSegments.push_back(std::move(seg));
				literal.clear();
			}

			p += 2;
			const char* keyStart = p;
			while (*p && *p != '}') ++p;

			std::string key(keyStart, p);
			if (*p == '}') ++p;

			auto it = s_keyToIdMap.find(key);
			if (it != s_keyToIdMap.end()) {
				CompiledSegment seg;
				seg.kind = CompiledSegment::Metric;
				seg.metricId = static_cast<RDMetricId>(it->second);
				seg.metricType = metaTable[it->second].type;
				outSegments.push_back(std::move(seg));
			} else {
				CompiledSegment seg;
				seg.kind = CompiledSegment::Literal;
				seg.literal = "${" + key + "}";
				seg.metricId = RDMetricId::COUNT;
				seg.metricType = RDMetricType::Int;
				outSegments.push_back(std::move(seg));

				auto logger = spdlog::get("RD");
				if (logger) {
					logger->warn("Unknown metric key in template: {}", key);
				}
			}
		} else {
			literal += *p;
			++p;
		}
	}

	if (!literal.empty()) {
		CompiledSegment seg;
		seg.kind = CompiledSegment::Literal;
		seg.literal = std::move(literal);
		seg.metricId = RDMetricId::COUNT;
		seg.metricType = RDMetricType::Int;
		outSegments.push_back(std::move(seg));
	}
}

static std::string _FormatIntWithCommas(int64_t value)
{
	if (value < 0) return "-" + _FormatIntWithCommas(-value);
	std::string s = std::to_string(value);
	int pos = static_cast<int>(s.length()) - 3;
	while (pos > 0) {
		s.insert(pos, ",");
		pos -= 3;
	}
	return s;
}

std::string RDDiagManager::_FormatCompiledTemplate(
	const std::vector<CompiledSegment>& segments)
{
	std::string result;
	for (const auto& seg : segments) {
		if (seg.kind == CompiledSegment::Literal) {
			result += seg.literal;
		} else {
			size_t idx = static_cast<size_t>(seg.metricId);
			switch (seg.metricType) {
			case RDMetricType::Int:
				result += _FormatIntWithCommas(s_metrics[idx].intVal);
				break;
			case RDMetricType::Float:
				{
					char buf[32];
					snprintf(buf, sizeof(buf), "%.1f", s_metrics[idx].floatVal);
					result += buf;
				}
				break;
			case RDMetricType::String:
				result += s_metrics[idx].strVal;
				break;
			}
		}
	}
	return result;
}
